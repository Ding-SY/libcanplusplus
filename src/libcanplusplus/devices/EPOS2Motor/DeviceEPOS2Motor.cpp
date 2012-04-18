/*!
 * @file 	DeviceEPOS2Motor.cpp
 * @brief
 * @author 	Christian Gehring
 * @date 	Jan, 2012
 * @version 1.0
 * @ingroup robotCAN, device
 *
 */

#include "DeviceEPOS2Motor.hpp"
#include <stdio.h>
#include <math.h>


DeviceEPOS2Motor::DeviceEPOS2Motor(int nodeId, DeviceEPOS2MotorParameters* deviceParams)
:Device(nodeId),deviceParams_(deviceParams)
{
	sdoStatusWord_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	sdoStatusWordDisabled_ = SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}

DeviceEPOS2Motor::~DeviceEPOS2Motor()
{
	if (deviceParams_ != NULL){
		delete deviceParams_;
	}

}


DeviceEPOS2MotorParameters* DeviceEPOS2Motor::getDeviceParams()
{
	return deviceParams_;
}

void DeviceEPOS2Motor::addRxPDOs()
{
	/* add Velocity RxPDO */
	rxPDOVelocity_ = new RxPDOVelocity(nodeId_, deviceParams_->rxPDOSMId_);
	bus_->getRxPDOManager()->addPDO(rxPDOVelocity_);
}

void DeviceEPOS2Motor::addTxPDOs()
{
	/* add PositionVelocity TxPDO */
	txPDOPositionVelocity_ = new TxPDOPositionVelocity(nodeId_, deviceParams_->txPDOSMId_);
	bus_->getTxPDOManager()->addPDO(txPDOPositionVelocity_);
}


void DeviceEPOS2Motor::setVelocity(double jointVelocity_rad_s)
{
	int motorVelocity_rpm =  (int) (jointVelocity_rad_s * deviceParams_->rad_s_Gear_to_rpm_Motor);
	rxPDOVelocity_->setVelocity(motorVelocity_rpm);
}

double DeviceEPOS2Motor::getPosition()
{
	return ((double) txPDOPositionVelocity_->getPosition()) / ( deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);

}

double DeviceEPOS2Motor::getVelocity()
{
	return ((double)txPDOPositionVelocity_->getVelocity()) / ( deviceParams_->rad_s_Gear_to_rpm_Motor);
}


void DeviceEPOS2Motor::setPositionLimits(double * positionLimit_rad)
{


	double minPositionLimit, maxPositionLimit;

	if (positionLimit_rad[0] < positionLimit_rad[1]) {
		minPositionLimit = positionLimit_rad[0];
		maxPositionLimit = positionLimit_rad[1];
	} else {
		minPositionLimit = positionLimit_rad[1];
		maxPositionLimit = positionLimit_rad[0];
	}

	int minLimit_ticks =  (int) (minPositionLimit * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);
	int maxLimit_ticks =  (int) (maxPositionLimit * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS);

	bus_->getSDOManager()->addSDO(new SDOSetMinPositionLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, minLimit_ticks));
	bus_->getSDOManager()->addSDO(new SDOSetMaxPositionLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, maxLimit_ticks));

}

void DeviceEPOS2Motor::setMotorParameters()
{

	SDOManager* SDOManager = bus_->getSDOManager();

	SDOManager->addSDO(new SDOSetEncoderPulseNumber(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->encoder_pulse_number));
	SDOManager->addSDO(new SDOSetPositionSensorType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->encoder_type));
	SDOManager->addSDO(new SDOSetPositionSensorPolarity(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->encoder_polarity, deviceParams_->hall_polarity));
	SDOManager->addSDO(new SDOSetMotorType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->motor_type));
	SDOManager->addSDO(new SDOSetPolePairNumber(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->pole_pair_number));
	SDOManager->addSDO(new SDOSetThermalTimeConstantWinding(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->thermal_time_constant_winding));

	SDOManager->addSDO(new SDOSetContinuousCurrentLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, (int)(deviceParams_->continuous_current_limit*1000.0)));
	SDOManager->addSDO(new SDOSetOutputCurrentLimit(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, (int)(deviceParams_->continuous_current_limit*1000.0)));

	/* set gains */
	SDOManager->addSDO(new SDOSetVelocityPGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->velocity_P_Gain));
	SDOManager->addSDO(new SDOSetVelocityIGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->velocity_I_Gain));
	SDOManager->addSDO(new SDOSetVelocityVelFFGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->velocity_VFF_Gain));
	SDOManager->addSDO(new SDOSetVelocityAccFFGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->velocity_AFF_Gain));

	SDOManager->addSDO(new SDOSetCurrentPGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->current_P_Gain));
	SDOManager->addSDO(new SDOSetCurrentIGain(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->current_I_Gain));

//	SDOManager->addSDO(new SDOSetMaxProfileVelocity(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_,  (int)(deviceParams_->max_profile_velocity *  deviceParams_->rad_s_Gear_to_rpm_Motor) ));
//	SDOManager->addSDO(new SDOSetProfileAcceleration(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_,  (int)(deviceParams_->profile_acceleration *  deviceParams_->rad_s_Gear_to_rpm_Motor) ));
//	SDOManager->addSDO(new SDOSetProfileDeceleration(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_,  (int)(deviceParams_->profile_decceleration *  deviceParams_->rad_s_Gear_to_rpm_Motor) ));

	SDOManager->addSDO(new SDOSetMaxFollowingError(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, (int)(deviceParams_->max_following_error * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS) ));

	SDOManager->addSDO(new SDOSetGuardTime(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0)); //Set a guard time of x ms with a factor , if set_guard_time(0): Guarding disabled
	SDOManager->addSDO(new SDOSetLifeTimeFactor(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 1));

	setPositionLimits(deviceParams_->positionLimits);


	SDOManager->addSDO(new SDOSetOperationMode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->operationMode));

	// 1=Fault signal only instead of Quickstop
	SDOManager->addSDO(new SDOSetAbortConnectionOptionCode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01));

}


bool DeviceEPOS2Motor::initDevice()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	SDOManager->addSDO(new SDONMTEnterPreOperational(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSetCOBIDSYNC(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x80));


	configTxPDOs();
	configRxPDOs();

	setMotorParameters();
	initMotor();


	SDOManager->addSDO(new SDONMTStartRemoteNode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	return true;

}

void DeviceEPOS2Motor::configTxPDOs()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	/* deactivate all TxPDOs */
	SDOManager->addSDO(new SDOTxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO2SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDOTxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));

	configTxPDOPositionVelocity();
}

void DeviceEPOS2Motor::configRxPDOs()
{

	SDOManager* SDOManager = bus_->getSDOManager();

	/* deactivate all RxPDOs */
	SDOManager->addSDO(new SDORxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO2SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));


	configRxPDOVelocity();
}

void DeviceEPOS2Motor::configTxPDOPositionVelocity()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	/* Transmit PDO 1 Parameter */

	///< configure COB-ID Transmit PDO 1
	SDOManager->addSDO(new SDOTxPDO1ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDOTxPDO1SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01)); // SYNC
	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Mapping "Position actual value"
	SDOManager->addSDO(new SDOTxPDO1SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x60640020));
	///< Mapping "Velocity"
	SDOManager->addSDO(new SDOTxPDO1SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x606C0020));
	///< Number of Mapped Application Objects
	SDOManager->addSDO(new SDOTxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02));

}

void DeviceEPOS2Motor::configRxPDOVelocity()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	/* Receive PDO 1 Parameter */
	///< Step 1: Configure COB-ID of the RxPDO 3
	SDOManager->addSDO(new SDORxPDO1ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Step 2: Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDORxPDO1SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01)); // SYNC
	///< Step 3: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Step 4: Mapping Objects
	///< Mapping " Velocity Setting Value"
	SDOManager->addSDO(new SDORxPDO1SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x206B0020));
	///< Mapping "Operation Mode"
	SDOManager->addSDO(new SDORxPDO1SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x60600008));
	///< Step 5: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02));
}

void DeviceEPOS2Motor::initMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDOFaultReset(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOShutdown(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSwitchOn(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOEnableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}


void DeviceEPOS2Motor::setEnableMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDOFaultReset(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOShutdown(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSwitchOn(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOEnableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}

void DeviceEPOS2Motor::setDisableMotor()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	SDOManager->addSDO(new SDODisableOperation(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
}


bool DeviceEPOS2Motor::getIsMotorEnabled(bool &flag)
{
	SDOManager* SDOManager = bus_->getSDOManager();


	if (!sdoStatusWord_->hasTimeOut()) {
		if (!sdoStatusWord_->getIsReceived()) {
			if (!sdoStatusWord_->getIsWaiting()) {
				if (!sdoStatusWord_->getIsQueuing()) {
					SDOManager->addSDO((SDOMsgPtr)sdoStatusWord_);
				}
			}
		} else {
			sdoStatusWord_->isEnabled(flag);
			sdoStatusWord_.reset();
			sdoStatusWord_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
			return true;
		}
	}

	return false;

}

bool DeviceEPOS2Motor::getIsMotorDisabled(bool &flag)
{
	SDOManager* SDOManager = bus_->getSDOManager();


	if (!sdoStatusWordDisabled_->hasTimeOut()) {
		if (!sdoStatusWordDisabled_->getIsReceived()) {
			if (!sdoStatusWordDisabled_->getIsWaiting()) {
				if (!sdoStatusWordDisabled_->getIsQueuing()) {
					SDOManager->addSDO((SDOMsgPtr)sdoStatusWordDisabled_);
				}
			}
		} else {
			sdoStatusWordDisabled_->isDisabled(flag);
			sdoStatusWordDisabled_.reset();
			sdoStatusWordDisabled_ =  SDOReadStatusWord::SDOReadStatusWordPtr(new SDOReadStatusWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
			return true;
		}
	}

	return false;

}

