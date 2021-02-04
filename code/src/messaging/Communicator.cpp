//
// Created by user on 10/17/20.
//

#include "Communicator.h"

namespace Messaging
{

    Communicator::Communicator(std::string baseIp, int port, unsigned char myId): Runnable(20.0f), _sockfdReceive(-1),
    _sockfdSend(-1), _myId(myId)
    {
        _listenDetails.ipAddr = baseIp; // Listen on all interfaces
        _listenDetails.port = port; // Our listen port

        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_ACK, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_HELLO, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_WELCOME, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
#if BASE_MODE
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_AGENT_MOVE_COMMAND, new thread_pool::static_pool(5)));
#else
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_AGENT_MOVE_COMMAND, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
#endif

#if BASE_MODE
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_AGENT_LOCATION, new thread_pool::static_pool(5)));
#else
        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_AGENT_LOCATION, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
#endif

        _threadPoolMap.emplace(std::make_pair(MessageID::MESSAGE_SUBJECT_LOCATION, new thread_pool::static_pool(CALLBACK_HANDLER_THREAD_COUNT)));
    }

    Communicator::~Communicator()
    {
        if(_sockfdReceive != -1)
        {
            close(_sockfdReceive);
            _sockfdReceive = -1;
        }
        if(_sockfdSend != -1)
        {
            close(_sockfdSend);
            _sockfdSend = -1;
        }
    }

    void Communicator::doSetup()
    {
        struct sockaddr_in servaddrSend, servaddrReceive;

        // Creating socket file descriptor
        if ( (_sockfdReceive = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Creating socket file descriptor
        if ( (_sockfdSend = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&servaddrSend, 0, sizeof(servaddrSend));
        memset(&servaddrReceive, 0, sizeof(servaddrReceive));

        // Filling server information

        // Setup for receiving port
        servaddrReceive = convertToCStruct(_listenDetails);

        // Setup for sending port
        CommDetails temp;
        temp = _listenDetails;
        temp.port = temp.port - 1;
        servaddrSend = convertToCStruct(temp);

        struct timeval read_timeout;
        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 10;
        setsockopt(_sockfdReceive, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(timeval));

        // Bind the socket with the server address
        if ( bind(_sockfdReceive, (const struct sockaddr *)&servaddrReceive, sizeof(sockaddr_in)) < 0 )
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Bind the socket with the server address
        if ( bind(_sockfdSend, (const struct sockaddr *)&servaddrSend, sizeof(sockaddr_in)) < 0 )
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
    }

    void Communicator::doRun()
    {
        int length = recv(
                _sockfdReceive,
                (char *)_buffer,
                MAXLINE,
                MSG_WAITFORONE
                );

        if(length > 0)
        {
            unsigned long timeNow = Utils::Time::microsNow();
            if(_lastTimeReceived )

            auto header = (Header *) _buffer;

            if (header->message_id == MessageID::MESSAGE_HELLO)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_HELLO);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_HELLO)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else if (header->message_id == MessageID::MESSAGE_WELCOME)
            {
                // Cast Message
                auto welcome = (Welcome*) _buffer;

                // SAve off our ID
                _myId = welcome->node_id;
                _baseId = welcome->header.source_id;

                // Call callback
                auto callback = _callbacks.find(MessageID::MESSAGE_WELCOME);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_WELCOME)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else if (header->message_id == MessageID::MESSAGE_ACK)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_ACK);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_ACK)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else if (header->message_id == MessageID::MESSAGE_AGENT_LOCATION)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_AGENT_LOCATION);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_AGENT_LOCATION)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else if (header->message_id == MessageID::MESSAGE_AGENT_MOVE_COMMAND)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_AGENT_MOVE_COMMAND);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_AGENT_MOVE_COMMAND)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else if (header->message_id == MessageID::MESSAGE_SUBJECT_LOCATION)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_SUBJECT_LOCATION);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPoolMap.find(MessageID::MESSAGE_SUBJECT_LOCATION)->second->enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
                }
            }
            else
            {
                std::cout << "Invalid message received with ID " << header->message_id << std::endl;
            }
        }
    }

    void Communicator::doStop()
    {
        if(_sockfdReceive != -1)
        {
            close(_sockfdReceive);
            _sockfdReceive = -1;
        }
        if(_sockfdSend != -1)
        {
            close(_sockfdSend);
            _sockfdSend = -1;
        }
    }

    void Communicator::fillHeader(unsigned char targetId, MessageID messageIdEnum, Header *header)
    {
        header->message_id = messageIdEnum;
        header->target_id = targetId;
        header->source_id = _myId;
        header->timestamp = Utils::Time::microsNow();
    }

    // Private
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, CommDetails &commDetails)
    {
        // TODO: Send the message
        Ack message;
        fillHeader(targetId, MessageID::MESSAGE_ACK, &message.header);
        message.message_id = messageIdAcked;
        message.acked_message_timestamp = messageTimestamp;
        message.ack_bool = acked;
        sendMessage(commDetails, (char*)&message, sizeof(Ack));
        return false;
    }

    // public
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked)
    {
        auto commStruct = _nodes.find(targetId);
        if(commStruct == _nodes.end())
        {
            return false;
        }
        return sendAck(targetId, messageIdAcked, messageTimestamp, acked, commStruct->second);
    }

    // public
    bool Communicator::sendAck(char targetId, char messageIdAcked, unsigned long messageTimestamp, bool acked, std::string ipAddr, int port)
    {
        // Retain the network details for this node ID
        CommDetails details;
        auto commStruct = _nodes.find(targetId);
        if(commStruct == _nodes.end())
        {
            details.ipAddr = std::move(ipAddr);
            details.port = port;
            _nodes.emplace(std::make_pair(targetId, details));
        }
        else
        {
            details = commStruct->second;
        }
        return sendAck(targetId, messageIdAcked, messageTimestamp, acked, details);
    }

    void Communicator::registerCallback(Messaging::Messages::Common::MessageID messageId,
                                        std::function<void(char *)> callback)
    {
        auto callbackFound = _callbacks.find(messageId);
        if(callbackFound == _callbacks.end())
        {
            _callbacks.emplace(std::make_pair(messageId, callback));
        }
        else
        {
            // It already exists
            _callbacks[messageId] = callback;
        }
    }

    void Communicator::callbackWrapper(std::function<void(char *)> func, char *buffer)
    {
        // Call the callback
        func(buffer);

        // Delete the memory after callback
        delete[] buffer;
    }

    void Communicator::sendMessage(CommDetails &commDetails, char *message, int length)
    {
        struct sockaddr_in servaddr = convertToCStruct(commDetails);
        int ret = sendto(
                _sockfdSend,
                (const char *)message,
                length,
                0,
                (const struct sockaddr *) &servaddr,
                sizeof(sockaddr_in)
        );
        if(ret < 0)
            std::cout << "Error sending message " << (int)((Header*)message)->message_id << std::endl;
    }

    sockaddr_in Communicator::convertToCStruct(Communicator::CommDetails &commDetails)
    {
        struct sockaddr_in servaddr{};
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family    = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = inet_addr(commDetails.ipAddr.c_str());
        servaddr.sin_port = htons(commDetails.port);
        return servaddr;
    }

}

