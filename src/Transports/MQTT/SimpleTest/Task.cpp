//***************************************************************************
// Copyright 2007-2020 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Faculdade de Engenharia da             *
// Universidade do Porto. For licensing terms, conditions, and further      *
// information contact lsts@fe.up.pt.                                       *
//                                                                          *
// Modified European Union Public Licence - EUPL v.1.1 Usage                *
// Alternatively, this file may be used under the terms of the Modified     *
// EUPL, Version 1.1 only (the "Licence"), appearing in the file LICENCE.md *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://github.com/LSTS/dune/blob/master/LICENCE.md and                  *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Eivind Jølsgard                                                 *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>
#include <mqtt/async_client.h>

namespace Transports
{
  //! Insert short task description here.
  //!
  //! Insert explanation on task behaviour here.
  //! @author Eivind Jølsgard
  namespace MQTT
  {
    namespace SimpleTest
    {
      using DUNE_NAMESPACES;

            //test
      const std::string DEFAULT_SERVER_ADDRESS	{ "tcp://localhost:2312" }; //TODO Change this for server
      const std::string DEFAULT_CLIENT_ID		    { "client 1"};



	    mqtt::connect_options conn_opts;
      //mqtt::async_client mqtt_client_test(DEFAULT_SERVER_ADDRESS, DEFAULT_CLIENT_ID	);
      mqtt::async_client* mqtt_client; // = &mqtt_client_test;
      auto timeout = std::chrono::seconds(10);

       struct Arguments
      {
        Address address; // Server address.
        int port; // Server port.
        std::string subscribe_topic;  //Topic to subscribe
        std::string publish_topic;  //Topic to publish
        std::string client_id;
        std::string lwt_payload; //Last will testament payload
        int  QOS; //Quality of service
      };

      struct Task: public DUNE::Tasks::Task
      {
        // Task arguments.
        Arguments m_args;
        // Parser handle.
        IMC::Parser m_parser;
        DUNE::Utils::ByteBuffer m_buf;
        //! Constructor.
        //! @param[in] name task name.
        //! @param[in] ctx context.
        Task(const std::string& name, Tasks::Context& ctx):
          DUNE::Tasks::Task(name, ctx)
        {

          param("Address", m_args.address)
          .defaultValue("tcp://localhost")
          .description("MQTT broker address");

          param("Subscribe topic", m_args.subscribe_topic)
          .defaultValue("toServer")
          .description("MQTT Subscribe topic");

          param("Port", m_args.port)
          .defaultValue("2023")
          .description("MQTT broker port");

          param("Publish topic", m_args.publish_topic)
          .defaultValue("toVeichle")
          .description("MQTT Publish topic");

          param("ClientID", m_args.client_id)
          .defaultValue("Default client ID")
          .description("MQTT Client ID");

          param("LWT payload", m_args.lwt_payload)
          .defaultValue("LWT payload")
          .description("MQTT Last Will Testament");

          param("QOS", m_args.QOS)
          .defaultValue("1")
          .description("MQTT Quality of service");
        }

        //! Update internal state with new parameter values.
        void
        onUpdateParameters(void)
        {
        }

        //! Reserve entity identifiers.
        void
        onEntityReservation(void)
        {
        }

        //! Resolve entity names.
        void
        onEntityResolution(void)
        {
        }

        //! Acquire resources.
        void
        onResourceAcquisition(void)
        {
          inf(DTR("connecting to MQTT broker"));
          std::string adr_string(m_args.address.c_str()); 
          adr_string.append(":");
          adr_string.append(std::to_string(m_args.port));
          inf(adr_string.c_str());
          mqtt_client = new mqtt::async_client(adr_string, m_args.client_id);

          conn_opts.set_keep_alive_interval(200); //MQTT?
	        conn_opts.set_clean_session(true);     //MQTT?
          try
          {
            mqtt_client->connect(conn_opts)->wait();
		        mqtt_client->start_consuming();
		        mqtt_client->subscribe(m_args.subscribe_topic, m_args.QOS)->wait();

            inf(DTR("connected to MQTT broker"));
            setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);
          }
          catch (std::runtime_error& e)
          {
            throw RestartNeeded(e.what(), 5);
          }
        }

        //! Initialize resources.
        void
        onResourceInitialization(void)
        {
        }

        //! Release resources.
        void
        onResourceRelease(void)
        {
          inf(DTR("onResourceRelease"));
          //TODO Disconnect
          try{
              if (mqtt_client)
              {
                auto toks = mqtt_client->get_pending_delivery_tokens();
                if (!toks.empty())
                inf(DTR("Error: There are pending MQTT delivery tokens!" ));
                
                mqtt_client->unsubscribe(m_args.subscribe_topic)->wait();
                mqtt_client->stop_consuming();
                mqtt_client->disconnect()->wait();
                delete mqtt_client;
              }
          }
          catch (std::runtime_error& e)
          {
            //throw RestartNeeded(e.what(), 5);
          }
        }

        //! Main loop.
        void
        onMain(void)
        {

          //Create IMC message
              IMC::PlanControl planControl;
        

              planControl.setTimeStamp(1.605363910147E9);
              planControl.setSource(65535);
              planControl.setSourceEntity(255);
              planControl.setDestination(26);  //26
              planControl.setDestinationEntity(46); //46
              planControl.plan_id = "s";
              planControl.flags = 0;
              planControl.op = 0;
              planControl.arg = {};

            /*  "timestamp": 1.605363910147E9,
  "src": 0,
  "src_ent": 0,
  "dst": 65535,
  "dst_ent": 255,
  "type": 0,
  "op": 0,
  "request_id": 0,
  "plan_id": "s",
  "flags": 0,
  "arg": {},
  "info": "" */

            planControl.toText(std::cout);
  


            unsigned int n = planControl.getSerializationSize();

            m_buf.grow(n);

            uint8_t* p = m_buf.getBuffer();

            IMC::Packet::serialize(&planControl, p, n);

            //Print serialized
            /*for (int i = 0; i < n; i++)
              {
                inf("%x", p[i]);
              }*/


            //send message  
            try
            {
              mqtt::message_ptr pubmsg = mqtt::make_message(m_args.publish_topic, p, n);
              pubmsg->set_qos(m_args.QOS);
              mqtt_client->publish(pubmsg)->wait_for(timeout);
            }
            catch (std::exception& e)
            {
              err(DTR("Transmission fail" ));
              throw RestartNeeded(e.what(), 5);
            }


          while (!stopping())
          {
            
            waitForMessages(1.0);
          }
        }
      };
    }
  }
}

DUNE_TASK
