#include <stdio.h>
#include <string.h>

#include "mSecExe.h"
#include "UttecLed.h"
#include "UttecUtil.h"
#include "CmdDefine.h"
#include "pirAnalog.h"
#include "volume.h"
#include "photoAnalog.h"

PwmOut dimer(p0);
UttecLed myLed;

DimmerRf* mSecExe::m_pRf = NULL;
UttecDim_t mSecExe::sDim = {0,};
//ePir, eVolume
eSensorType_t mSecExe::m_sensorType = ePir;

bool mSecExe::m_sensorFlag = false;

pirAnalog pirA;
volume vol;
photoAnalog photoA;

mSecExe::mSecExe(DimmerRf* pRf){
	Flash* myFlash;
	rfFrame_t* pyFrame = &myFlash->getFlashFrame()->rfFrame;

//	pyFrame->Ctr.SensorRate = 5;
	pirA.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
	vol.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
	photoA.setSensorRate(pyFrame->Ctr.DTime/1024.0);
	m_pRf = pRf;
	dimer.period_us(300);		//set Pwm Freq
//	dimer.period_us(25);		//set Pwm Freq
	dimer = 0.3;			//set Pwm initial duty
}

void mSecExe::procDim(UttecDim_t sDim){
//	putchar('.');
	static float fNow = 0;
	if(sDim.target >= fNow){
		fNow += sDim.upStep;
		if(fNow >= sDim.target) fNow = sDim.target;
	}
	else{
		fNow -= sDim.downStep;
		if(fNow <= sDim.target) fNow = sDim.target;
	}
	sDim.pwm = fNow;
	dimer = (float)1.0 - sDim.pwm; 
//	dimer = fNow;
}

void mSecExe::switchDimType(rfFrame_t* pFrame){
	if(sDim.forced){	//when forced Mode
		dimer = sDim.target;
		return;
	}
	switch(m_sensorType){
		case ePir:
//			putchar('1');
			if(sDim.dTime) sDim.dTime--;
			else{	//when Delay Time out
				sDim.target = (float)pFrame->Ctr.Low/(float)100.0;
//				putchar('.');
			}
			sDim.upStep = 0.005;
			sDim.downStep = 0.0003;
			sDim.forced = false;
			procDim(sDim);		
			break;
		case eVolume:
			sDim.upStep = 0.01;
			sDim.downStep = 0.01;
			sDim.forced = false;
			procDim(sDim);		
			break;
		case eDayLight:
			sDim.upStep = 0.01;
			sDim.downStep = 0.01;
			sDim.forced = false;
			procDim(sDim);		
			break;
		default:
			break;
	}
}

void mSecExe::switchSensorType(rfFrame_t* pFrame){
	static uint32_t ulTimeout = 0;
	if(ulTimeout) ulTimeout--;
	switch(pFrame->MyAddr.SensorType.iSensor){
		case ePir:
			m_sensorType = ePir;			
			if(pirA.procPirSensor(ePirAnalog)){
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
//					printf("Sens = %0.3f\n\r", sDim.target);
				}
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
			if(photoA.procPhotoA(ePhotoDigital)&&pFrame->MyAddr.RxTx.Bit.Tx){
				m_sensorType = eDayLight;		
				photoA.clearSensorFlag();	
				if(!ulTimeout){
				float delayTime = 20;  //20Sec	
				float delta = (1.0*TimeoutForPhotoRepeat)/(1000.0*delayTime);	
					if(photoA.getDir() == eUp){
						if(sDim.target < 1.0) 
							sDim.target = sDim.target + delta; //10Sec
							if(sDim.target > 1.0) sDim.target = 1.0;
					}
					else{
						if(sDim.target > 0.0) 
							sDim.target = sDim.target - delta;	//Max 10Sec
							if(sDim.target < 0.0) sDim.target = 0.0;
					}						
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
	static bool isRealMode = false;
	ulCount++;
	
//	putchar('.');
	if(!myUtil.isMstOrGw(pFrame)){
		if(isRealMode)
			switchSensorType(pFrame);
		switchDimType(pFrame);
	}
	/*
	*/
	myLed.taskLed();
	if(!(ulCount%500)){
		pFrame->Ctr.Level = sDim.target*100;
		dimFactors_t sFactors = {sDim.forced,m_sensorType,
			sDim.target,sDim.pwm}; 
		
		myUtil.getDimFactor(sFactors);
//		printf("getDimFactor\n\r");
		myLed.blink(eRfLed, eRfBlink);
		myLed.blink(eSensLed, eSensBlink);
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
