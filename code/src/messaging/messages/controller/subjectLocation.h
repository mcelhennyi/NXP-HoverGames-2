//
// Created by user on 10/17/20.
//

#ifndef HOVERGAMES2_SUBJECTLOCATION_H
#define HOVERGAMES2_SUBJECTLOCATION_H

#include "../common/header.h"
#include "../common/location.h"

namespace Messaging
{
    namespace Messages
    {
        namespace Controller
        {
            enum SubjectType
            {
                SUBJECT_FRIENDLY = 1,
                SUBJECT_NEUTRAL,
                SUBJECT_SUSPECT,
                SUBJECT_HOSTILE
            };

            struct SubjectLocation
            {
                Common::Header      header;
                unsigned char       subject_id;         // The ID of the subject
                unsigned char       type_id;            // The SubjectType enum of the subject
                unsigned char       _padding[6];        // Padding for alignment

                Common::Location    current_location;   // The current location of the subject
            };
        }
    }
}


#endif //HOVERGAMES2_SUBJECTLOCATION_H
