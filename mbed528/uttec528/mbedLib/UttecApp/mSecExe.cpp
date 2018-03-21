#include <stdio.h>
#include <string.h>

#include "mSecExe.h"
#include "UttecLed.h"
#include "UttecUtil.h"
#include "CmdDefine.h"
#include "pirAnalog.h"
#include "volume.h"
#include "photoAnalog.h"
#include "eprom.h"

PwmOut dimer(p0);
UttecLed myLed;

//static eprom testEp();


DimmerRf* mSecExe::m_pRf = NULL;
UttecDim_t mSecExe::sDim = {0,};
//ePir, eVolume
eSensorType_t mSecExe::m_sensorType = ePir;

bool mSecExe::m_sensorFlag = false;

pirAnalog pirA;
volume vol;
photoAnalog photoA;

mSecExe::mSecExe(){
}

mSecExe::mSecExe(DimmerRf* pRf){
	Flash* myFlash;
	rfFrame_t* pyFrame = &myFlash->getFlashFrame()->rfFrame;

//	pyFrame->Ctr.SensorRate = 5;
	pirA.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
	vol.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
	photoA.setSensorRate(0.2);
//	photoA.setSensorRate(pyFrame->Ctr.DTime/1024.0);
	m_pRf = pRf;
	dimer.period_us(300);		//set Pwm Freq
//	dimer.period_us(25);		//set Pwm Freq
	dimer = 0.3;			//set Pwm initial duty
}

void mSecExe::procDim(){
	eprom testEp;

//	putchar('.');
	static float fNow = 0;
	bool bAct = false;
	
	if(sDim.target >= fNow){
		fNow += sDim.upStep;
		if(fNow >= sDim.target) fNow = sDim.target;
		else bAct = true;
	}
	else{
		fNow -= sDim.downStep;
		if(fNow <= sDim.target) fNow = sDim.target;
		else bAct = true;
	}
	sDim.pwm = fNow;
	sDim.current = fNow;
	
	if(bAct){
		dimer = (float)1.0 - sDim.pwm; 
		testEp.writeByte02(0, 5);
	}
//	dimer = fNow;
}

void mSecExe::switchDimType(rfFrame_t* pFrame){
	static uint32_t ulCount = 0;
	if(sDim.forced){	//when forced Mode
		if((ulCount++%20)) return;
		dimer = sDim.target;
		sDim.pwm = sDim.target;
		return;
	}
	switch(m_sensorType){
		case ePir:
			if(sDim.dTime) sDim.dTime--;
			else{	//when Delay Time out
				sDim.target = (float)pFrame->Ctr.Low/(float)100.0;
			}
			sDim.upStep = 0.005;
			sDim.downStep = 0.0003;
			sDim.forced = false;
			procDim();		
			break;
		case eVolume:
			sDim.upStep = 0.01;
			sDim.downStep = 0.01;
			sDim.forced = false;
			procDim();		
			break;
		case eTestMode:	
			sDim.upStep = 0.0001;
			sDim.downStep = 0.0001;
			sDim.forced = false;
//			putchar('1');
			procDim();		
			break;
		case eDayLight:
			sDim.upStep = 0.0001;
			sDim.downStep = 0.0001;
			sDim.forced = false;
			procDim();		
			break;
		default:
			break;
	}
}
#include "monitor.h"

void mSecExe::switchSensorType(rfFrame_t* pFrame){
	monitor myMon;
	static uint32_t ulTimeout = 100;
	if(ulTimeout) ulTimeout--;
	switch(pFrame->MyAddr.SensorType.iSensor){
		case ePir:
			m_sensorType = ePir;			
			if(pirA.procPirSensor(ePirAnalog)){
				myLed.blink(eSensLed, eSensBlink);
//			if(pirA.procPirSensor(ePirDigital)){
				pirA.clearSensorFlag();
				if(!ulTimeout){
					ulTimeout = TimeoutForPirRepeat; //0.5Sec
					sDim.target = pFrame->Ctr.High/100.0;
					sDim.dTime = pFrame->Ctr.DTime*1000;
					printf("\n\rFrom Pir:");
//					printf("\n\r***************target = %0.3f\n\r", sDim.target);
					pFrame->Cmd.Command = edSensor;
					setSensorFlag();
					myMon.setTrafficFlag();
//					printf("Sens = %0.3f\n\r", sDim.target);
				}
			}
			break;
		case eTestMode:	
				m_sensorType = eTestMode;		
				if(!ulTimeout){
					ulTimeout=500;
//					printf("pwm = %f, %f\r\n",sDim.target, sDim.current);
				}
			break;
		case eVolume:
			if(pFrame->MyAddr.RxTx.Bit.Tx){
							pFrame->Ctr.Level = sDim.target*100;
			}
			if(vol.procVolumeSw()&&pFrame->MyAddr.RxTx.Bit.Tx){
				sDim.target = vol.getTarget();
				m_sensorType = eVolume;		
				vol.clearSensorFlag();	
				if(!ulTimeout){
					ulTimeout = TimeoutForVolumeRepeat; //0.2Sec
					setSensorFlag();
					pFrame->Ctr.Level = sDim.target*100;
					pFrame->Cmd.Command = edVolume;
					putchar('v');
//					printf("------- Vol = %0.3f\n\r", sDim.target);
				}
			}
			break;
		case eDayLight:
			if(pFrame->MyAddr.RxTx.Bit.Tx){
							pFrame->Ctr.Level = sDim.target*100;
			}
			if(photoA.procPhotoA(ePhotoAnalog)&&pFrame->MyAddr.RxTx.Bit.Tx){
//			if(photoA.procPhotoA(ePhotoDigital)&&pFrame->MyAddr.RxTx.Bit.Tx){
				m_sensorType = eDayLight;		
				photoA.clearSensorFlag();	
//				putchar('.');
//				printf("photoA \r\n");
				if(!ulTimeout){
//					printf("&&&&&&&&&&&&ulTimeout:%d\r\n", ulTimeout);
				float delayTime = 60;  //20Sec	
				float delta = (1.0*TimeoutForPhotoRepeat)/(1000.0*delayTime);	
					if(photoA.getDir() == eUp){
						printf("--Up\r\n");
						if(sDim.target < 1.0) 
							sDim.target = sDim.target + delta; //10Sec
							if(sDim.target > 1.0) sDim.target = 1.0;
					}
					else{
						printf("--Down\r\n");
						if(sDim.target > 0.0) 
							sDim.target = sDim.target - delta;	//Max 10Sec
							if(sDim.target < 0.0) sDim.target = 0.0;
					}	
					printf("pwm = %f\r\n", sDim.current);	
					ulTimeout = TimeoutForPhotoRepeat; //0.5Sec	
					setSensorFlag();
					pFrame->Ctr.Level = sDim.target*100;
					pFrame->Cmd.Command = edDayLight;
					printf("\n\rFrom Pir:");
					/*
					printf("------- Vol = %0.3f, photo = %f\n\r", 
						sDim.target, photoA.m_sPhotoA.current);
					*/
				}
			}
			break;
		default:
			break;
	}
}

void mSecExe::msecTask(rfFrame_t* pFrame){
	UttecUtil myUtil;	
	static uint32_t ulCount = 0;	
	static bool isRealMode = true;
	
	ulCount++;
	
	if(!myUtil.isMstOrGw(pFrame)){
		if(isRealMode)
			switchSensorType(pFrame);
//		if(0) //for test 20180303
		switchDimType(pFrame);
	}
	myLed.taskLed();
	if(!(ulCount%500)){
//		testPwm();
//		testLed();
		pFrame->Ctr.Level = sDim.target*100;
		dimFactors_t sFactors = {sDim.forced,m_sensorType,
			sDim.target,sDim.current}; 
		myUtil.getDimFactor(sFactors);
//		printf("getDimFactor\n\r");
	}
}
bool mSecExe::returnSensorFlag(){
	return m_sensorFlag;
}
void mSecExe::clearSensorFlag(){
	m_sensorFlag = false;
}
void mSecExe::setSensorFlag(){
	m_sensorFlag = true;
}
/*
*/
void mSecExe::setForcedDim(float level){
	sDim.forced = true;
	sDim.target = level;
}
void mSecExe::setUnforcedDim(){
	sDim.forced = false;
}

void mSecExe::testLed(){
	static bool bToggel = true;
	bToggel = !bToggel;
	
	if(bToggel){
		myLed.blink(eRfLed, eRfBlink);
	}
	else{
		myLed.blink(eSensLed, eSensBlink);
	}
}

void mSecExe::testPwm(){
	static float fPwm = 0.0;
	setForcedDim(fPwm);	
	fPwm += 0.02;
	if(fPwm>1.0) fPwm = 0.0;
}

void mSecExe::setDirectDim(float dim){
	dimer = dim;
}

