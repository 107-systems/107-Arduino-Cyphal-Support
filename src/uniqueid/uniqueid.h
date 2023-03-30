/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2023 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-Cyphal-Support/graphs/contributors.
 */

#ifndef INC_107_ARDUINO_CYPHAL_SUPPORT_UNIQUEID_H
#define INC_107_ARDUINO_CYPHAL_SUPPORT_UNIQUEID_H

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <107-Arduino-UniqueId.h>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace cyphal::support
{

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class UniqueId : public impl::UniqueId16 { };

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* cyphal::support */

#endif /* INC_107_ARDUINO_CYPHAL_SUPPORT_UNIQUEID_H */
