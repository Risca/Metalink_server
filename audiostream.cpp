#include "audiostream.h"

bool myAudioStream::initialized = false;
MSSndCard * myAudioStream::sndcard_capture = 0;
RtpProfile * myAudioStream::rtp_profile = 0;

myAudioStream::myAudioStream() :
    filtersLinked(false)
{
    if (!initialized) {
//        ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);
        ortp_set_log_level_mask(ORTP_DEBUG|ORTP_ERROR);
        ms_init();
        ortp_init();
        sndcard_capture = init_card();
        rtp_profile = init_rtp_profile();
        initialized = true;
    }
    _payload = payloads().first();
}

myAudioStream::~myAudioStream()
{
}

void myAudioStream::start()
{
    init_stream();
    init_filters();
    link_filters();
    start_ticker();
}

void myAudioStream::stop()
{
    stop_ticker();
    unlink_filters();
    destroy_filters();
    /* Destroy RTP session */
    ms_message("Destroying RTP session");
    rtp_session_destroy(stream->session);
}

void myAudioStream::changePayload(const QString &payload)
{
    stop_ticker();
    unlink_filters();
    destroy_filters();

    setPayload(payload);

    init_filters();
    link_filters();
    start_ticker();
}

QStringList myAudioStream::payloads()
{
    if (!initialized) {
        return QStringList();
    }

    PayloadType *pt;

    /* List all available payloads */
    QStringList payloads;
    for (int i = 0; i < RTP_PROFILE_MAX_PAYLOADS; i++) {
        pt = rtp_profile_get_payload(rtp_profile,i);
        if (pt != 0) {
            QString payload(pt->mime_type);
            if (payloads.contains(payload)) {
                payload.append(" " + QString::number(pt->clock_rate));
            }
            payloads << payload;
        }
    }
    return payloads;
}

MSSndCard * myAudioStream::init_card()
{
    const MSList *list = ms_snd_card_manager_get_list(ms_snd_card_manager_get());
    QStringList cards;
    while (list != NULL) {
        cards << ms_snd_card_get_string_id((MSSndCard*)list->data);
        list = list->next;
    }
    if (cards.isEmpty()) {
        QMessageBox::information(0, QObject::tr("No cards detected!"), QObject::tr("No cards detected!"));
        return 0;
    }

    bool ok;
    QString card_id = QInputDialog::getItem(0, QObject::tr("Select capture card"),
                                            QObject::tr("Select capture card"),
                                            cards, 0, false, &ok);
    if (!ok) {
        return 0;
    }

    MSSndCard * sndcard = ms_snd_card_manager_get_card(ms_snd_card_manager_get(),
                                                               card_id.toStdString().c_str());
    if (sndcard==NULL){
        ms_error("No capture card found.");
        return 0;
    } else {
        ms_message("Got card: %s", ms_snd_card_get_string_id(sndcard));
    }

    return sndcard;
}

RtpProfile * myAudioStream::init_rtp_profile()
{
    /* Create profile to use in session */
    RtpProfile *profile = rtp_profile_new("My profile");
//    RtpProfile *rtp_profile = rtp_profile_clone_full(&av_profile);
    if (profile == 0) {
        ms_error("Failed to create new RTP profile");
        return 0;
    }

    /* Define some payloads */
    rtp_profile_set_payload(profile,110,&payload_type_speex_nb);
    rtp_profile_set_payload(profile,111,&payload_type_speex_wb);
    rtp_profile_set_payload(profile,112,&payload_type_speex_uwb);

    return profile;
}

bool myAudioStream::init_stream()
{
    /** Init stream **/
    stream = (AudioStream *)ms_new0(AudioStream,1);
    if (stream == 0) {
        ms_error("Failed to create new stream");
        return false;
    }

    /** Configure stream **/
    stream->play_dtmfs = false;
    stream->use_gc = false;
    stream->use_agc = false;
    stream->use_ng = false;

    /** Init RTP session **/
    stream->session = rtp_session_new(RTP_SESSION_SENDONLY);
    if (stream->session == 0) {
        ms_error("Failed to create new RTP session");
        return false;
    }

    /** Configure RTP session **/
    rtp_session_set_profile(stream->session, rtp_profile);

    /* Set remote address and port */
    const char *addr = _remoteIP.toString().toStdString().c_str();
    ms_message("Setting remote IP: %s", addr);
    rtp_session_set_remote_addr(stream->session, addr, 1337);
    return true;
}

bool myAudioStream::init_filters()
{
    /** Init filters **/
    stream->soundread = ms_snd_card_create_reader(sndcard_capture);
    RtpProfile *profile = rtp_session_get_profile(stream->session);
    PayloadType *pt;

    /* List all available payloads */
    QMap<QString,int> payloads;
    for (int i = 0; i < RTP_PROFILE_MAX_PAYLOADS; i++) {
        pt = rtp_profile_get_payload(profile,i);
        if (pt != 0) {
            QString payload(pt->mime_type);
            if (payloads.contains(payload)) {
                payload.append(" " + QString::number(pt->clock_rate));
            }
            payloads.insert(payload,i);
        }
    }

    int payload_type_number = payloads.value(_payload);
    pt = rtp_profile_get_payload(profile,payload_type_number);

    stream->encoder = ms_filter_create_encoder(pt->mime_type);
    stream->rtpsend=ms_filter_new(MS_RTP_SEND_ID);

    /** Configure filter options **/
    /* Set payload type to use when sending */
    rtp_session_set_payload_type(stream->session, payload_type_number);
    /* Set session used by rtpsend */
    ms_filter_call_method(stream->rtpsend,MS_RTP_SEND_SET_SESSION,stream->session);

    int sr = pt->clock_rate;
    if (ms_filter_call_method(stream->soundread, MS_FILTER_SET_SAMPLE_RATE, &sr) !=0 ) {
        ms_error("Problem setting sample rate on soundread filter!");
        return false;
    }

    if (ms_filter_call_method(stream->encoder, MS_FILTER_SET_SAMPLE_RATE, &sr) != 0) {
        ms_error("Problem setting sample rate on encoder filter!");
        return false;
    }
    ms_message("Payload's bitrate is %i",sr);
    if (pt->normal_bitrate>0){
        ms_message("Setting audio encoder network bitrate to %i",sr);
        ms_filter_call_method(stream->encoder,MS_FILTER_SET_BITRATE,&sr);
    }

    return true;
}

bool myAudioStream::link_filters()
{
    /** Create chain (soundread --> encoder --> rtpsend) **/
    /* Link filters together */
    if (ms_filter_link(stream->soundread,0,stream->encoder,0) != 0) {
        ms_error("Failed to link soundread --> encoder");
        return false;
    }
    if (ms_filter_link(stream->encoder,0,stream->rtpsend,0) != 0) {
        ms_error("Failed to link encoder --> rtpsend");
        return false;
    }
    filtersLinked = true;
    return true;
}

bool myAudioStream::start_ticker()
{
    /* Create ticker */
    ticker = ms_ticker_new();
    if (ticker == 0) {
        ms_error("Failed to create new ticker");
        return false;
    }
    /* Attach chain to ticker */
    if (ms_ticker_attach(ticker, stream->soundread) != 0) {
        ms_error("Failed to attach soundread to ticker");
        return false;
    }
    return true;
}

QString myAudioStream::getStatus()
{
    if (stream == 0) {
        return "No stream!";
    }

    if (stream->session){
        QString str;
        QTextStream(&str) << "Bandwidth usage: download=" <<
                rtp_session_compute_recv_bandwidth(stream->session)*1e-3 <<
                " kbits/sec, upload=" <<
                rtp_session_compute_send_bandwidth(stream->session)*1e-3 <<
                " kbits/sec\n";
        return str;
    } else  {
        return "No session!";
    }
}

void myAudioStream::stop_ticker()
{
    /* Detach ticker */
    if (ticker) {
        ms_message("Detaching ticker");
        if (ms_ticker_detach(ticker, stream->soundread) != 0) {
            ms_error("Failed to detach ticker");
        }
    }
    /* Destroy ticker */
    ms_message("Destroying ticker");
    if (ticker!=NULL) ms_ticker_destroy(ticker);
}

void myAudioStream::unlink_filters()
{
    /* Unlink chain */
    if (filtersLinked) {
        ms_message("Unlinking chain");
        if (ms_filter_unlink(stream->soundread,0,stream->encoder,0) != 0) {
            ms_error("Failed to unlink soundread --> encoder");
        }
        if (ms_filter_unlink(stream->encoder,0,stream->rtpsend,0) != 0) {
            ms_error("Failed to unlink encoder --> rtpsend");
        }
    }
}

void myAudioStream::destroy_filters()
{
    /* Destroy filters */
    ms_message("Destroying filters");
    if (stream->rtpsend != NULL) ms_filter_destroy(stream->rtpsend);
    if (stream->soundread != NULL) ms_filter_destroy(stream->soundread);
    if (stream->encoder != NULL) ms_filter_destroy(stream->encoder);
}
