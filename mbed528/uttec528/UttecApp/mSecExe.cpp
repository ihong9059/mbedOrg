#include <stdio.h>
#include <string.h>

#include "mSecExe.h"
#include "UttecLed.h"
#include "UttecUtil.h"
#include "CmdDefine.h"
#include "pirAnalog.h"
#include "volume.h"
#include "photoAnalog.h"
#include "dac.h"

PwmOut dimer(p0);
UttecLed myLed;

DimmerRf* mSecExe::m_pRf = NULL;
UttecDim_t mSecExe::sDim = {0,};
//ePir, eVolume
eSensorType_t mSecExe::m_sensorType = ePir;

bool mSecExe::m_sensorFlag = false;
uint32_t mSecExe::m_noiseBlockTime = 0;

pirAnalog pirA;
volume vol;
photoAnalog photoA;

void mSecExe::setNoiseBlockTime(uint32_t ulTime){
	m_noiseBlockTime = ulTime;
}

mSecExe::mSecExe(){
}

mSecExe::mSecExe(DimmerRf* pRf){
	Flash* myFlash;
	rfFrame_t* pyFrame = &myFlash->getFlashFrame()->rfFrame;

//	pyFrame->Ctr.SensorRate = 5;
	pirA.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
	vol.setSensorRate(pyFrame->Ctr.SensorRate/100.0);
//	photoA.setSensorRate(0.2);
	photoA.setSensorRate(pyFrame->Ctr.DTime/1024.0);
	m_pRf = pRf;
	dimer.period_us(3000);		//set Pwm Freq
//	dimer.period_us(300);		//set Pwm Freq
//	dimer.period_us(25);		//set Pwm Freq
	dimer = 0.3;			//set Pwm initial duty
}

void mSecExe::procDim(){
	dac testEp;

//	putchar('.');
	static float fNow = 0.5;
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
//		putchar('*');
		dimer = (float)1.0 - sDim.pwm; 
		if(testEp.m_enableFlag)
			testEp.writeDac(sDim.pwm);
	}
//	dimer = fNow;
}

void mSecExe::switchDimType(rfFrame_t* pFrame){
	dac testEp;
	static uint32_t ulCount = 0;
	static float fBefore = 0;
	if(sDim.forced){	//when forced Mode
		if(fBefore == sDim.target) return;
		if((ulCount++%20)) return;
		dimer = (float)1.0 - sDim.target;
		sDim.pwm = sDim.target;
		if(testEp.m_enableFlag)
			testEp.writeDac(sDim.pwm);
		fBefore = sDim.target;
		return;
	}
	switch(m_sensorType){
		case ePir:	
//			putchar('.');
			if(sDim.dTime) sDim.dTime--;
			else{	//when Delay Time out
//				putchar('*');
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
					pFrame->Cmd.Command = edSensor;
					setSensorFlag();
					myMon.setTrafficFlag();
				}
			}
			break;
		case eTestMode:	
				m_sensorType = eTestMode;		
				if(!ulTimeout){
					ulTimeout=500;
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
				if(!ulTimeout){
					if(photoA.getDir() == eUp){
						printf("--Up\r\n");
						if(sDim.target < 1.0) 
							sDim.target = sDim.target + 0.01; //10Sec
						else sDim.target = 1.0;
					}
					else{
						printf("--Down\r\n");
						if(sDim.target > 0.0) 
							sDim.target = sDim.target - 0.01;	//Max 10Sec
						else sDim.target = 0.0;
					}	
					printf("pwm = %f, target = %f\r\n", sDim.current, sDim.target);	
					ulTimeout = TimeoutForPhotoRepeat; //0.5Sec	
					setSensorFlag();
					pFrame->Ctr.Level = sDim.target*100;
					pFrame->Cmd.Command = edDayLight;
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

void mSecExe::msecTask(rfFrame_t* pFrame){	//mSecStart
	UttecUtil myUtil;	
	static uint32_t ulCount = 0;	
	static bool isRealMode = true;
	
	ulCount++;
	if(m_noiseBlockTime) m_noiseBlockTime--;
	
	if(!myUtil.isMstOrGw(pFrame)){
		if(isRealMode&&(!m_noiseBlockTime)&&(!sDim.forced))
			switchSensorType(pFrame);
		switchDimType(pFrame);
	}
	myLed.taskLed();
	if(!(ulCount%500)){
		pFrame->Ctr.Level = sDim.target*100;
		dimFactors_t sFactors = {sDim.forced,m_sensorType,
			sDim.target,sDim.current}; 
		myUtil.getDimFactor(sFactors);
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
	dac testEp;
	dimer = dim;
	if(testEp.m_enableFlag)
		testEp.writeDac(dim);
}

