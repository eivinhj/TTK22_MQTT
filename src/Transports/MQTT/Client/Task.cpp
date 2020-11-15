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
// Author: Eivind Jolsgard                                                  *
//***************************************************************************

// ISO C++ 98 headers.
#include <cstddef>

// DUNE headers.
#include <DUNE/DUNE.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <atomic>
#include <mqtt/async_client.h>



namespace Transports
{
  namespace MQTT
  {


    namespace Client
    {
      using DUNE_NAMESPACES;

	    mqtt::connect_options conn_opts;
      mqtt::async_client* mqtt_client; 
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

      struct Task: public Tasks::SimpleTransport
      {
        // Task arguments.
        Arguments m_args;
        // Socket handle.
        TCPSocket* m_sock;
        // Parser handle.
        IMC::Parser m_parser;

        Task(const std::string& name, Tasks::Context& ctx):
          Tasks::SimpleTransport(name, ctx),
          m_sock(NULL)
        {
          param("Address", m_args.address)
          .defaultValue("tcp://localhost")
          .description("MQTT broker address");

          param("Subscribe topic", m_args.subscribe_topic)
          .defaultValue("toVeichle")
          .description("MQTT Subscribe topic");

          param("Port", m_args.port)
          .defaultValue("2023")
          .description("MQTT broker port");

          param("Publish topic", m_args.publish_topic)
          .defaultValue("toServer")
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

        ~Task(void)
        {
          onResourceRelease();
        }

        void
        onResourceAcquisition(void)
        {
          inf(DTR("connecting to MQTT broker"));

          //Concatenate adress and port
          std::string adr_string(m_args.address.c_str()); 
          adr_string.append(":");
          adr_string.append(std::to_string(m_args.port));
          inf(adr_string.c_str());

          //Create mqtt_client instance
          mqtt_client = new mqtt::async_client(adr_string, m_args.client_id);

          conn_opts.set_keep_alive_interval(200); 
	        conn_opts.set_clean_session(true);     
          try
          {
            //Connect and subscribe to topic
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
                err(DTR("Error: There are pending MQTT delivery tokens!" ));
                
                mqtt_client->unsubscribe(m_args.subscribe_topic)->wait();
                mqtt_client->stop_consuming();
                mqtt_client->disconnect()->wait();
                delete mqtt_client;
              }

               
          }
        catch (std::runtime_error& e)
          {
            throw RestartNeeded(e.what(), 5);
          }
          
          m_parser.reset();
        }

        void
        onDataTransmission(const uint8_t* p, unsigned int len)
        {
          try
          {
            mqtt::message_ptr pubmsg = mqtt::make_message(m_args.publish_topic, p, len);
            pubmsg->set_qos(m_args.QOS);
             
            mqtt_client->publish(pubmsg)->wait_for(timeout);
          }
          catch (std::exception& e)
          {
            err(DTR("Transmission failiure" ));
            throw RestartNeeded(e.what(), 5);
          }
        }

        void
        onDataReception(uint8_t* p, unsigned int n, double timeout)
        {
          (void) timeout;

          auto msg = mqtt_client->consume_message();
			    if (!msg) return;
			    inf(DTR("Receiving message" ));
          unsigned int n_r;
           

          try
          {
            if(msg->get_topic() == m_args.subscribe_topic){

              //Print topic
              std::string topic_inf = "Topic is ";
              topic_inf.append(msg->get_topic());
              inf(topic_inf.c_str());
            
              p = (uint8_t*) msg->get_payload().data();
              n_r = msg->get_payload().size();

              if (n_r > n)
              {
                err("MQTT Message length is longer than data reception buffer"); 
              }

            }
            else{
              inf(DTR("Topic of no interrest" ));
              
              return; 
            }
          }
          catch (std::exception& e)
          {
            throw RestartNeeded(e.what(), 5);
          }

          if (n_r > 0)
            handleData(m_parser, (uint8_t*)p, n_r);
        }
      };
    }
  }
}

DUNE_TASK
