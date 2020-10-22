import struct

# Written/Copied on 10/21/2020

# See src/messaging/messages for the structs to match the below codes

# enum MessageID
# {
#     // Common
#     MESSAGE_ACK=1,
#
#     // Node ORIGINATING message (Agent or Controller)
#     MESSAGE_HELLO,
#
#     // Base Station ORIGINATING messages
#     MESSAGE_WELCOME,
#
#     // Controller ORIGINATING Messages
#     MESSAGE_AGENT_MOVE_COMMAND,
#     MESSAGE_SUBJECT_LOCATION,
#
#     // Agent ORIGINATING Messages
#     MESSAGE_AGENT_LOCATION
#
# };

MESSAGE_ACK = 1
MESSAGE_HELLO = 2
MESSAGE_WELCOME = 3
MESSAGE_AGENT_MOVE_COMMAND = 4
MESSAGE_SUBJECT_LOCATION = 5
MESSAGE_AGENT_LOCATION = 6


# struct Header
# {
#     unsigned char   message_id;     // The message's id
#     unsigned char   source_id;      // The Source of the message
#     unsigned char   target_id=0;    // The target of the message - 255 for broadcast
#     unsigned char   _padding[5];    // Unused - for alignment
#     unsigned long   timestamp;      // This message creation time
# };
header = "BBBxxxxxL"

# struct Ack
# {
#     Header          header;
#     unsigned long   acked_message_timestamp;    // The timestamp of the acked message
#     unsigned char   message_id;                 // The acked message
#     unsigned char   ack_bool;                   // 0 for false, anything else for true
# };
ack = "LBBB"

# enum NodeType
# {
#     NODE_TYPE_BASE = 0,
#     NODE_TYPE_AGENT,
#     NODE_TYPE_CONTROLLER
# };
NODE_TYPE_BASE = 0
NODE_TYPE_AGENT = 1
NODE_TYPE_CONTROLLER = 2

# struct IPAddress
# {
#     unsigned char ip1;
#     unsigned char ip2;
#     unsigned char ip3;
#     unsigned char ip4;
# };
ip_address = "BBBB"

# struct Hello
# {
#     Header header;
#     unsigned char node_type;          // NodeType Enum above
#     unsigned short listeningPort;     // The IP port this device listens on
#     unsigned char _padding[5];
#     IPAddress address;                // The IP address of this device
# };
hello = "BHxxxxx" + ip_address

# struct Location
# {
#     float x;
#     float y;
#     float z;
#     float _padding;
# };
location = "fffxxxx"

# struct Welcome
# {
#     Header header;
#     unsigned char node_id;    // The assigned ID
# };
welcome = "B"

# struct AgentMoveCommand
# {
#     Common::Header      header;
#     unsigned char       agent_id;           // The ID of the agent
#     unsigned char       _padding[7];        // Padding for alignment
#
#     Common::Location    target_location;    // The target location for the agent
# };
agent_move_command = "BB" + location

# enum SubjectType
# {
#     SUBJECT_FRIENDLY = 1,
#     SUBJECT_NEUTRAL,
#     SUBJECT_SUSPECT,
#     SUBJECT_HOSTILE
# };
SUBJECT_FRIENDLY = 1
SUBJECT_NEUTRAL = 2
SUBJECT_SUSPECT = 3
SUBJECT_HOSTILE = 4

# struct SubjectLocation
# {
#     Common::Header      header;
#     unsigned char       subject_id;         // The ID of the subject
#     unsigned char       type_id;            // The SubjectType enum of the subject
#     unsigned char       _padding[6];        // Padding for alignment
#
#     Common::Location    current_location;   // The current location of the subject
# };
subject_location = "BBxxxxxx" + location

# struct AgentLocation
# {
#     Common::Header      header;
#     unsigned char       agent_id;           // The ID of the agent
#     unsigned char       owner_id;           // The owner of the agent
#     unsigned char       _padding[6];        // Padding for alignment
#
#     Common::Location    current_location;   // The current location of the agent
#     Common::Location    target_location;    // The target location of the agent
# };
agent_location = "BBxxxxxx" + location + location


class Message:
    def __init__(self, raw_bytes):
        # pull off the header
        self._header_tuple = struct.unpack(header, raw_bytes)

        self._raw_bytes = raw_bytes

    def unpack(self, fmt):
        return struct.unpack_from(fmt, self._raw_bytes, struct.calcsize(header))  # TODO might need +1

    def get_message_id(self):
        return self._header_tuple[0]
