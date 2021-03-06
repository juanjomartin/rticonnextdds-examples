/* profiles_subscriber.cxx

   A subscription example

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C++ -example <arch> profiles.idl

   Example subscription of type profiles automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example publication.

   (2) Start the subscription with the command
       objs/<arch>/profiles_subscriber <domain_id> <sample_count>

   (3) Start the publication with the command
       objs/<arch>/profiles_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
              
                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       On Unix: 
       
       objs/<arch>/profiles_publisher <domain_id> 
       objs/<arch>/profiles_subscriber <domain_id> 
                            
       On Windows:
       
       objs\<arch>\profiles_publisher <domain_id>  
       objs\<arch>\profiles_subscriber <domain_id>   
              
       
modification history
------------ -------       
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef RTI_VX653
#include <vThreadsData.h>
#endif
#include "profiles.h"
#include "profilesSupport.h"
#include "ndds/ndds_cpp.h"

#define PROFILE_NAME_STRING_LEN 100

class profilesListener : public DDSDataReaderListener {
  public:
    profilesListener(char listener_name[PROFILE_NAME_STRING_LEN])
    {
        strcpy(_listener_name, listener_name);
    }
    virtual void on_requested_deadline_missed(
        DDSDataReader* /*reader*/,
        const DDS_RequestedDeadlineMissedStatus& /*status*/) {}
    
    virtual void on_requested_incompatible_qos(
        DDSDataReader* /*reader*/,
        const DDS_RequestedIncompatibleQosStatus& /*status*/) {}
    
    virtual void on_sample_rejected(
        DDSDataReader* /*reader*/,
        const DDS_SampleRejectedStatus& /*status*/) {}

    virtual void on_liveliness_changed(
        DDSDataReader* /*reader*/,
        const DDS_LivelinessChangedStatus& /*status*/) {}

    virtual void on_sample_lost(
        DDSDataReader* /*reader*/,
        const DDS_SampleLostStatus& /*status*/) {}

    virtual void on_subscription_matched(
        DDSDataReader* /*reader*/,
        const DDS_SubscriptionMatchedStatus& /*status*/) {}

    virtual void on_data_available(DDSDataReader* reader);
  private:
    char _listener_name[PROFILE_NAME_STRING_LEN];
};

void profilesListener::on_data_available(DDSDataReader* reader)
{
    profilesDataReader *profiles_reader = NULL;
    profilesSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    DDS_ReturnCode_t retcode;
    int i;

    profiles_reader = profilesDataReader::narrow(reader);
    if (profiles_reader == NULL) {
        printf("DataReader narrow error\n");
        return;
    }

    retcode = profiles_reader->take(
        data_seq, info_seq, DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);

    if (retcode == DDS_RETCODE_NO_DATA) {
        return;
    } else if (retcode != DDS_RETCODE_OK) {
        printf("take error %d\n", retcode);
        return;
    }

    printf("=============================================\n");
    printf("%s listener received\n", _listener_name);
    printf("=============================================\n");
    for (i = 0; i < data_seq.length(); ++i) {
        if (info_seq[i].valid_data) {
            profilesTypeSupport::print_data(&data_seq[i]);
        }
    }

    retcode = profiles_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        printf("return loan error %d\n", retcode);
    }
}

/* Delete all entities */
static int subscriber_shutdown(
    DDSDomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides the finalize_instance() method on
       domain participant factory for people who want to release memory used
       by the participant factory. Uncomment the following block of code for
       clean destruction of the singleton. */
/*
    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }
*/
    return status;
}

extern "C" int subscriber_main(int domainId, int sample_count)
{
    DDSDomainParticipant *participant = NULL;
    DDSSubscriber *subscriber = NULL;
    DDSTopic *topic = NULL;
    /* Volatile and transient local readers and profilesListeners */
    DDSDataReader *reader_volatile = NULL;
    DDSDataReader *reader_transient_local = NULL;
    profilesListener *reader_volatile_listener = NULL;
    profilesListener *reader_transient_local_listener = NULL;
    DDS_ReturnCode_t retcode;
    const char *type_name = NULL;
    int count = 0;
    DDS_Duration_t receive_period = {1,0};
    int status = 0;

    /* There are several different approaches for loading QoS profiles from XML
     * files (see Configuring QoS with XML chapter in the RTI Connext Core
     * Libraries and Utilities User's Manual). In this example we illustrate
     * two of them:
     *
     * 1) Creating a file named USER_QOS_PROFILES.xml, which is loaded,
     * automatically by the DomainParticipantFactory. In this case, the file
     * defines a QoS profile named volatile_profile that configures reliable,
     * volatile DataWriters and DataReaders.
     *
     * 2) Adding XML documents to the DomainParticipantFactory using its
     * Profile QoSPolicy (DDS Extension). In this case, we add
     * my_custom_qos_profiles.xml to the url_profile sequence, which stores
     * the URLs of all the XML documents with QoS policies that are loaded by
     * the DomainParticipantFactory aside from the ones that are automatically
     * loaded.
     * my_custom_qos_profiles.xml defines a QoS profile named
     * transient_local_profile that configures reliable, transient local
     * DataWriters and DataReaders.
     */

    /* To load my_custom_qos_profiles.xml, as explained above, we need to modify
     * the  DDSTheParticipantFactory Profile QoSPolicy */
    DDS_DomainParticipantFactoryQos factory_qos;
    DDSTheParticipantFactory->get_qos(factory_qos);

    /* We are only going to add one XML file to the url_profile sequence, so we
     * ensure a length of 1,1. */
    factory_qos.profile.url_profile.ensure_length(1, 1);

    /* The XML file will be loaded from the working directory. That means, you
     * need to run the example like this:
     * ./objs/<architecture>/profiles_publisher
     * (see README.txt for more information on how to run the example).
     *
     * Note that you can specify the absolute path of the XML QoS file to avoid
     * this problem.
     */
    factory_qos.profile.url_profile[0] =
            DDS_String_dup("my_custom_qos_profiles.xml");

    DDSTheParticipantFactory->set_qos(factory_qos);


    /* Our default Qos profile, volatile_profile, sets the participant name.
     * This is the only participant_qos policy that we change in our
     * example. As this is done in the default QoS profile, we don't need
     * to specify its name, so we can create the participant using the
     * create_participant() method rather than using
     * create_participant_with_profile().  */
    participant = DDSTheParticipantFactory->create_participant(
        domainId, DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* We haven't changed the subscriber_qos in any of QoS profiles we use in
     * this example, so we can just use the create_topic() method. If you want
     * to load an specific profile in which you may have changed the
     * publisher_qos, use the create_publisher_with_profile() method. */
    subscriber = participant->create_subscriber(
        DDS_SUBSCRIBER_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        printf("create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Register the type before creating the topic */
    type_name = profilesTypeSupport::get_type_name();
    retcode = profilesTypeSupport::register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

    /* We haven't changed the topic_qos in any of QoS profiles we use in this
     * example, so we can just use the create_topic() method. If you want to
     * load an specific profile in which you may have changed the topic_qos,
     * use the create_topic_with_profile() method. */
    topic = participant->create_topic(
        "Example profiles",
        type_name, DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Create Data Readers listeners */
    reader_volatile_listener =
            new profilesListener("volatile_profile");
    reader_transient_local_listener =
            new profilesListener("transient_local_profile");

    /* Volatile reader -- As volatile_profile is the default qos profile
     * we don't need to specify the profile we are going to use, we can
     * just call create_datareader passing DDS_DATAWRITER_QOS_DEFAULT. */
    reader_volatile = subscriber->create_datareader(
        topic, DDS_DATAREADER_QOS_DEFAULT, reader_volatile_listener,
        DDS_STATUS_MASK_ALL);
    if (reader_volatile == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        delete reader_volatile_listener;
        return -1;
    }

    /* Transient Local writer -- In this case we use
     * create_datareader_with_profile, because we have to use a profile other
     * than the default one. This profile has been defined in
     * my_custom_qos_profiles.xml, but since we already loaded the XML file
     * we don't need to specify anything else. */
    reader_transient_local = subscriber->create_datareader_with_profile(
            topic, /* DDS_Topic* */
            "profiles_Library", /* library_name */
            "transient_local_profile", /* profile_name */
            reader_transient_local_listener /* listener */,
            DDS_STATUS_MASK_ALL /* DDS_StatusMask */
            );
    if (reader_transient_local == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        delete reader_transient_local_listener;
        return -1;
    }
    printf("Created reader_transient_local");

    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {
        NDDSUtility::sleep(receive_period);
    }

    /* Delete all entities */
    status = subscriber_shutdown(participant);
    delete reader_volatile_listener;
    delete reader_transient_local_listener;

    return status;
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t** argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */ 
    
    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }
    
    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
                                  
    return subscriber_main(domainId, sample_count);
}

#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }


    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
                                  
    return subscriber_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char* __ctype = *(__ctypePtrGet());

extern "C" void usrAppInit ()
{
#ifdef  USER_APPL_INIT
    USER_APPL_INIT;         /* for backwards compatibility */
#endif
    
    /* add application specific code here */
    taskSpawn("sub", RTI_OSAPI_THREAD_PRIORITY_NORMAL, 0x8, 0x150000, (FUNCPTR)subscriber_main, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   
}
#endif

