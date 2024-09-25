/*
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "DataExchangeTypeSupportImpl.h"

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <dds/DCPS/StaticIncludes.h>
#if OPENDDS_DO_MANUAL_STATIC_INCLUDES
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <ace/Log_Msg.h>
#include <random>


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  try {
    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    // Create DomainParticipant
    DDS::DomainParticipant_var participant =
      dpf->create_participant(42,
                              PARTICIPANT_QOS_DEFAULT,
                              0,
                              OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!participant) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_participant failed!\n")),
                       1);
    }

    // Register TypeSupport (Sensors::SoundingData)
    Sensors::SoundingDataTypeSupport_var ts =
      new Sensors::SoundingDataTypeSupportImpl;

    if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" register_type failed!\n")),
                       1);
    }

    // Create Topic (Sonar XYZ)
    CORBA::String_var type_name = ts->get_type_name();
    DDS::Topic_var topic_sounding =
      participant->create_topic("Sonar XYZ",
                                type_name,
                                TOPIC_QOS_DEFAULT,
                                0,
                                OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic_sounding) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_topic failed!\n")),
                       1);
    }

    // Create Publisher
    DDS::Publisher_var publisher =
      participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                    0,
                                    OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!publisher) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_publisher failed!\n")),
                       1);
    }

    // Create DataWriter
    DDS::DataWriter_var writer_sounding =
      publisher->create_datawriter(topic_sounding,
                                   DATAWRITER_QOS_DEFAULT,
                                   0,
                                   OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!writer_sounding) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_datawriter_sounding failed!\n")),
                       1);
    }

    Sensors::SoundingDataDataWriter_var sounding_writer =
      Sensors::SoundingDataDataWriter::_narrow(writer_sounding);

    if (!sounding_writer) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" _narrow failed!\n")),
                       1);
    }




    // Block until Subscriber is available
    DDS::StatusCondition_var condition_sounding = writer_sounding->get_statuscondition();
    condition_sounding->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

    DDS::WaitSet_var ws = new DDS::WaitSet;
    ws->attach_condition(condition_sounding);


    ACE_DEBUG((LM_DEBUG,
               ACE_TEXT("Block until subscriber is available\n")));

    while (true) {
      DDS::PublicationMatchedStatus matches;
      if (writer_sounding->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" get_publication_matched_status _sounding failed!\n")),
                         1);
      }

      if (matches.current_count >= 1) {
        break;
      }

      DDS::ConditionSeq conditions;
      DDS::Duration_t timeout = { 60, 0 };
      if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" wait failed!\n")),
                         1);
      }
    }

    ACE_DEBUG((LM_DEBUG,
               ACE_TEXT("Subscriber is available\n")));

    ws->detach_condition(condition_sounding);

    // Write samples
    Sensors::SoundingData soundingdata;
    soundingdata.sensor_id = 99;

    soundingdata.depthBelowTransducer = 100;
    soundingdata.depthBelowKeel       = 50;
    soundingdata.validity             = true;

    // Generate some randon number to add some dynamics to the values
    // Create a random number generator engine
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create a distribution that generates numbers in the range [-1, 1]
    std::uniform_real_distribution<> dis(-1.0, 1.0); 


    for (int i = 0; i < 10; ++i) {
      DDS::ReturnCode_t error = sounding_writer->write(soundingdata, DDS::HANDLE_NIL);
      ++soundingdata.sensor_id;
      soundingdata.depthBelowTransducer +=  dis(gen)*10 ;
      soundingdata.depthBelowKeel       +=  dis(gen)*5 ;

      if (error != DDS::RETCODE_OK) {
        ACE_ERROR((LM_ERROR,
                   ACE_TEXT("ERROR: %N:%l: main() -")
                   ACE_TEXT(" write returned %d!\n"), error));
      }
    }

    // Wait for samples to be acknowledged
    DDS::Duration_t timeout = { 30, 0 };

    if (sounding_writer->wait_for_acknowledgments(timeout) != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" wait_for_acknowledgments failed!\n")),
                       1);
    }

    
    // Clean-up!
    participant->delete_contained_entities();
    dpf->delete_participant(participant);

    TheServiceParticipant->shutdown();

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return 1;
  }

  return 0;
}
