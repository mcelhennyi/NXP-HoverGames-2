//
// Created by user on 10/17/20.
//

#include "Communicator.h"

namespace Messaging
{
    Communicator::Communicator(char myId): Runnable(20.0f), _sockfd(-1), _myId(myId), _threadPool(CALLBACK_HANDLER_THREAD_COUNT)
    {

    }

    Communicator::~Communicator()
    {
        if(_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    void Communicator::doSetup()
    {
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        if ( (_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr.sin_family    = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(PORT);

        struct timeval read_timeout;
        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 10;
        setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

        // Bind the socket with the server address
        if ( bind(_sockfd, (const struct sockaddr *)&servaddr,
                  sizeof(servaddr)) < 0 )
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
    }

    void Communicator::doRun()
    {
        int length = recv(
                _sockfd,
                (char *)_buffer,
                MAXLINE,
                MSG_WAITFORONE
                );

        if(length > 0)
        {
            auto header = (Header *) _buffer;

            if (header->message_id == MessageID::MESSAGE_HELLO)
            {
                auto callback = _callbacks.find(MessageID::MESSAGE_HELLO);
                if (callback != _callbacks.end())
                {
                    // Call the callback with the received message
                    auto copied = new char[length];
                    memcpy(copied, _buffer, length);
                    _threadPool.enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
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
                    _threadPool.enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
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
                    _threadPool.enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
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
                    _threadPool.enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
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
                    _threadPool.enqueue(std::bind(&Communicator::callbackWrapper, this, callback->second, copied));
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
        if(_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    void Communicator::fillHeader(char targetId, MessageID messageIdEnum, Header *header)
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
        func(buffer);
        delete[] buffer;
    }

    void Communicator::sendMessage(CommDetails &commDetails, char *message, int length)
    {
        struct sockaddr_in servaddr{};
        servaddr.sin_addr.s_addr = inet_addr(commDetails.ipAddr.c_str());
        servaddr.sin_port = commDetails.port;
        int ret = sendto(
                _sockfd,
                (const char *)message,
                length,
                0,
                (const struct sockaddr *) &servaddr,
                sizeof(sockaddr_in)
        );
        if(ret < 0)
            std::cout << "Error sending message " << (int)((Header*)message)->message_id << std::endl;
    }

}

