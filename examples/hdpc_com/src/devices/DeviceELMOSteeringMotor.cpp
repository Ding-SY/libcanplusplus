/*!
 * @file 	DeviceELMOSteeringMotor.cpp
 * @brief
 * @author 	Christian Gehring
 * @date 	Jan, 2012
 * @version 1.0
 * @ingroup robotCAN, device
 *
 */

#include "DeviceELMOSteeringMotor.hpp"
#include <stdio.h>
#include <math.h>


DeviceELMOSteeringMotor::DeviceELMOSteeringMotor(int nodeId, DeviceELMOMotorParametersHDPC* deviceParams)
:DeviceELMOBaseMotor(nodeId,deviceParams)
{

}

DeviceELMOSteeringMotor::~DeviceELMOSteeringMotor()
{

}


void DeviceELMOSteeringMotor::addRxPDOs()
{

	/* add Position RxPDO */
	rxPDOPosition_ = new RxPDOPosition(nodeId_, deviceParams_->rxPDO1SMId_);
	bus_->getRxPDOManager()->addPDO(rxPDOPosition_);

	/* add PP Profile Velocity RxPDO */ //NOT WORKING
	//rxPDOPPProfileVelocity_ = new RxPDOPPProfileVelocity(nodeId_, deviceParams_->rxPDO2SMId_);
	//bus_->getRxPDOManager()->addPDO(rxPDOPPProfileVelocity_);

}

void DeviceELMOSteeringMotor::setProfilePosition(double jointPosition_rad)
{

	int jointPosition_ticks = (jointPosition_rad - deviceParams_->homeOffsetJointPosition_rad) * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS;
	rxPDOPosition_->setPosition(jointPosition_ticks);

	/* debugging */
	//jointPosition_ticks = 500000;
	//	Working with elmo
	//	SDOManager* SDOManager = bus_->getSDOManager();
	//	SDOManager->addSDO(new SDOSetProfilePosition(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, jointPosition_ticks));
	//SDOManager->addSDO(new SDOControlWord(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x03F));
}

//NOT WORKING!!
/*void DeviceELMOSteeringMotor::setPPProfileVelocity(double jointVelocity_rad_s)
{

	int jointVelocity_ticks_s = jointVelocity_rad_s * deviceParams_->gearratio_motor * deviceParams_->RAD_TO_TICKS;
	rxPDOPPProfileVelocity_->setPPProfileVelocity(jointVelocity_ticks_s);
}*/

void DeviceELMOSteeringMotor::setMotorParameters()
{
	SDOManager* SDOManager = bus_->getSDOManager();
	setPositionLimits(deviceParams_->positionLimits);

	SDOManager->addSDO(new SDOSetOperationMode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->operationMode));
	SDOManager->addSDO(new SDOSetDS402ConfigurationObject(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02)); //0x02
	SDOManager->addSDO(new SDOSetPPModeProfileVelocity(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, deviceParams_->profileVelocityInPPMode)); //Set profile velocity to 1000RPM

}


bool DeviceELMOSteeringMotor::initDevice()
{
	SDOManager* SDOManager = bus_->getSDOManager();

//	SDOManager->addSDO(new SDONMTResetCommunication(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
//	SDOManager->addSDO(new SDONMTResetNode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));


//	printf("NMT: Enter Pre-Operational\n");
	SDOManager->addSDO(new SDONMTEnterPreOperational(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	SDOManager->addSDO(new SDOSetCOBIDSYNC(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x80));



	/* configure the PDOs on the motor controller */
	configTxPDOs();
	configRxPDOs();

	/* configure several motor parameters */
	setMotorParameters();
	initMotor();


//	printf("NMT: Start remote node\n");
	SDOManager->addSDO(new SDONMTStartRemoteNode(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));

	return true;

}


void DeviceELMOSteeringMotor::configRxPDOs()
{

	SDOManager* SDOManager = bus_->getSDOManager();

	/* deactivate all RxPDOs */
	SDOManager->addSDO(new SDORxPDO1SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO2SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	SDOManager->addSDO(new SDORxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));

	configRxPDOProfilePosition();
	//configRxPDOPPProfileVelocity(); //NOT WORKING
}


void DeviceELMOSteeringMotor::configRxPDOProfilePosition()
{
	SDOManager* SDOManager = bus_->getSDOManager();

	/* Receive PDO 3 Parameter */
	///< Step 1: Configure COB-ID of the RxPDO 3
	SDOManager->addSDO(new SDORxPDO3ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Step 2: Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDORxPDO3SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00)); // SYNC
	///< Step 3: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Step 4: Mapping Objects

	///< Mapping "Operation Mode"
	SDOManager->addSDO(new SDORxPDO3SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x60600008));

	///< Mapping "Target position"
	SDOManager->addSDO(new SDORxPDO3SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x02, 0x607A0020));
	///< Mapping "Controlword"
	SDOManager->addSDO(new SDORxPDO3SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x03, 0x60400010));


	///< Step 5: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO3SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x03));
}

/*
//NOT WORKING
void DeviceELMOSteeringMotor::configRxPDOPPProfileVelocity()
//{
	SDOManager* SDOManager = bus_->getSDOManager();

	// Receive PDO 4 Parameter
	///< Step 1: Configure COB-ID of the RxPDO 4
	SDOManager->addSDO(new SDORxPDO4ConfigureCOBID(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_));
	///< Step 2: Set Transmission Type: SYNC 0x01
	SDOManager->addSDO(new SDORxPDO4SetTransmissionType(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00)); // SYNC
	///< Step 3: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x00));
	///< Step 4: Mapping Objects

	///< Mapping "Profile Velocity"
	SDOManager->addSDO(new SDORxPDO4SetMapping(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01, 0x60810020));


	///< Step 5: Number of Mapped Application Objects
	SDOManager->addSDO(new SDORxPDO4SetNumberOfMappedApplicationObjects(deviceParams_->inSDOSMId_, deviceParams_->outSDOSMId_, nodeId_, 0x01));
}*/

void DeviceELMOSteeringMotor::setEnableMotor()
{
	rxPDOPosition_->enable();
}

void DeviceELMOSteeringMotor::setDisableMotor()
{
	rxPDOPosition_->disable();
}



