#include "config.h"


aaConfig::aaConfig(char *paramfile)
{
	m_pFSConfig = new cv::FileStorage();
	m_pCameraInputParams = new aaCameraInputParams;

	if(m_pFSConfig == nullptr)
	{
		CV_Assert(0);
	}

	if(m_pCameraInputParams == nullptr)
	{
		CV_Assert(0);
	}

	parseParams(paramfile);

}

int convertbinary2decimal(int input[] , int count)
{
	int mode = 0;

	for(int i= 0 ; i < count ; i++)
	{
		mode += input[count-1-i]*pow(2,i);

	}

	return mode;

}

string aaCamInfo::getCurrentStateString()
{

	switch(currentState)
	{

		case STATE_SERVE :

			return "STATE_SERVE";

		case STATE_PREBOUNCE :

			return "STATE_PREBOUNCE";

		case STATE_POSTBOUNCE :

			return "STATE_POSTBOUNCE";


		case STATE_POSTRACKETHIT :

			return "STATE_POSTRACKETHIT";

		case STATE_POSTNETCROSS :

			return "STATE_POSTNETCROSS";

	}


}



string aaCamInfo::getCurrentEventString()
{

	switch(currentEvent)
	{

		case EVENT_NONE :

			return "EVENT_NONE";

		case EVENT_BOUNCE :

			return "EVENT_BOUNCE";

		case EVENT_RACKETHIT :

			return "EVENT_RACKETHIT";


		case EVENT_NETCROSS :

			return "EVENT_NETCROSS";

		case EVENT_NETHIT :

			return "EVENT_NETHIT";


	}


}

stateName aaCamInfo::getCurrentState()
{

	return currentState;


}



eventName aaCamInfo::getCurrentEvent()
{
	return currentEvent;

}

void aaConfig::parseParams( char *paramfile)
{
	//cv::FileStorage  * const pFSConfig = getpConfig();
	//aaCameraInputParams  *  const pCameraInputParams = getpInputParams();


	bool openResult = m_pFSConfig->open(paramfile, cv::FileStorage::READ);

	if(!openResult)
	{
		CV_Assert(0);
	}

	struct timeval tp1;
    	gettimeofday(&tp1, NULL);

	char prefix[100];
	sprintf(prefix ,"./output/%d", tp1.tv_sec);

	cout<<"Timestamp for output dump : "<< prefix <<endl;


	const cv::FileNode &rootFileNode = m_pFSConfig->root();

	const cv::FileNode &system = rootFileNode["AceActSystem"]["system"];

	system["enableTwoCameras"] >> m_pCameraInputParams->enableTwoCameras;
	system["enableLiveCapture"] >> m_pCameraInputParams->enableLiveCapture;
	system["numberOfCameras"] >> m_pCameraInputParams->cameraNum;
	system["startCam"] >> m_pCameraInputParams->startCam;
	system["startCamID"] >> m_pCameraInputParams->startCamId;
	system["enableStartCamID"] >> m_pCameraInputParams->enableStartCamId;

	system["enableSystemOutput"] >> m_pCameraInputParams->systemOutputInfo.enableOutput;
	
	string tempsystemoutfiledir;
	
	system["system_outfiledir"] >>tempsystemoutfiledir ;

	char systemoutfiledir[100];

	sprintf(systemoutfiledir ,"%s%s", prefix, tempsystemoutfiledir.c_str());
	
	m_pCameraInputParams->systemOutputInfo.outFiledir = systemoutfiledir;

	system["system_outfilename"] >> m_pCameraInputParams->systemOutputInfo.outFilename;
	system["system_outtype"] >> m_pCameraInputParams->systemOutputInfo.outputType;
	system["system_ostartframe"] >> m_pCameraInputParams->systemOutputInfo.startFrame;
	system["system_oendframe"] >> m_pCameraInputParams->systemOutputInfo.endFrame;




	m_pCameraInputParams->systemCurrentFrame = m_pCameraInputParams->systemOutputInfo.startFrame;

	const cv::FileNode &cameras = rootFileNode["AceActSystem"]["cameras"];
	const cv::FileNode &displayparams = rootFileNode["AceActDisplay"]["nuc"]; // will change for Tegra
	const cv::FileNode &bemparams = rootFileNode["AceActBallEventManager"]["models"];
	const cv::FileNode &ppparams = rootFileNode["AceActPlayerPoseDetection"]["opose"];
	const cv::FileNode &odparams = rootFileNode["AceActObjectDetection"]["camera_od_params"];
	const cv::FileNode &ttparams = rootFileNode["AceActTrackingParameters"]["toss_tracker_module"];
	const cv::FileNode &ptparams = rootFileNode["AceActTrackingParameters"]["projectile_tracker_module"];

	const cv::FileNode &liveparams = rootFileNode["AceActSystem"]["live"];

	const int count = m_pCameraInputParams->cameraNum;

	int numEnabledCams = 0;

	int enableMode[count];


	for(int i = 0; i < count; ++i)
	{

		const cv::FileNode &camera = cameras["camera" + to_string(i)];
		const cv::FileNode &displayparam = displayparams["camera" + to_string(i)];
		const cv::FileNode &bemparam = bemparams["camera" + to_string(i)];
		const cv::FileNode &ppparam = ppparams["camera" + to_string(i)];
		const cv::FileNode &odparam = odparams["camera" + to_string(i)];
		const cv::FileNode &ttparam = ttparams["camera" + to_string(i)];
		const cv::FileNode &ptparam = ptparams["camera" + to_string(i)];
		const cv::FileNode &liveparam = liveparams["camera" + to_string(i)];


//		int enableRun;

//		camera["enableRun"+ to_string(i)]>> enableRun;

//		enableMode[i] = enableRun;

//		if(!enableRun)
//		{
//			continue;
//		}

		numEnabledCams++;

		aaCamInfo cameraInfo;

		liveparam["inwidth"+ to_string(i)] >> cameraInfo.liveParams.inputVideoInfo.width;
		liveparam["inheight"+ to_string(i)] >> cameraInfo.liveParams.inputVideoInfo.height;
		liveparam["infps"+ to_string(i)] >> cameraInfo.liveParams.inputVideoInfo.fps;
		liveparam["informat"+ to_string(i)] >> cameraInfo.liveParams.inputVideoInfo.inputFormat;

		liveparam["instartframe"+ to_string(i)] >> cameraInfo.liveParams.instartframe;
		liveparam["inendframe"+ to_string(i)] >> cameraInfo.liveParams.inendframe;

		liveparam["outrecord"+ to_string(i)] >> cameraInfo.liveParams.record;
		liveparam["enableoutput"+ to_string(i)] >> cameraInfo.liveParams.outputVideoInfo.enableOutput;
		//liveparam["outfiledir"] >> cameraInfo.liveParams.outputVideoInfo.outFiledir;

		string templiveoutfiledir;

		liveparam["outfiledir"+ to_string(i)] >> templiveoutfiledir;

		char liveoutfiledir[100];

		sprintf(liveoutfiledir ,"%s%s", prefix, templiveoutfiledir.c_str());
	
		cameraInfo.liveParams.outputVideoInfo.outFiledir = liveoutfiledir;

		//liveparam["outfilename"] >> cameraInfo.liveParams.outputVideoInfo.outFilename;

		string templiveoutfilename;

		liveparam["outfilename"+ to_string(i)] >> templiveoutfilename;
		
		char liveoutfilename[100];

		sprintf(liveoutfilename ,"%s/%s", liveoutfiledir, templiveoutfilename.c_str());

	 	cameraInfo.liveParams.outputVideoInfo.outFilename = liveoutfilename;

		liveparam["outtype"+ to_string(i)] >> cameraInfo.liveParams.outputVideoInfo.outputType;
		liveparam["ostartframe"+ to_string(i)] >> cameraInfo.liveParams.outputVideoInfo.startFrame;
		liveparam["oendframe"+ to_string(i)] >> cameraInfo.liveParams.outputVideoInfo.endFrame;
		liveparam["outpreview"+ to_string(i)] >> cameraInfo.liveParams.preview;
	
		camera["enableScreenPrint"+ to_string(i)] >>  cameraInfo.enableScreenPrint;

		camera["enableLogFilePrint"+ to_string(i)] >>  cameraInfo.enableLogFilePrint;

		camera["enableRenderFrame"+ to_string(i)] >> cameraInfo.enableRenderFrame;

		camera["enableRecord"+ to_string(i)] >> cameraInfo.enableRecord;

		camera["camId"+ to_string(i)] >> cameraInfo.camId;
		
		if( cameraInfo.camId == m_pCameraInputParams->startCamId)
		{
			cameraInfo.enableRun = 1;
		}
		else
		{
			cameraInfo.enableRun = 0;
		}
				

		string templogoutputPath;

		camera["logoutputPath"+ to_string(i)] >>  templogoutputPath;

		char logoutputPath[100];

		sprintf(logoutputPath ,"%s%s", prefix, templogoutputPath.c_str());
	
		cameraInfo.logDirName = logoutputPath;

		camera["logoutfilename"+ to_string(i)] >>  cameraInfo.logFileName;


//		camera["enableRun"+ to_string(i)]>> cameraInfo.enableRun;

//		cameraInfo.enableRun = enableRun;

		camera["camname"+ to_string(i)]>>cameraInfo.name;

		camera["infile"+ to_string(i)] >> cameraInfo.streamName;

		string tempinFiledir;
		string tempinFilename;


		camera["infiledir"+ to_string(i)] >> tempinFiledir;
		camera["infilename"+ to_string(i)] >> tempinFilename;


		char infiledir[100];

		sprintf(infiledir ,"%s%s",  tempinFiledir.c_str(), cameraInfo.streamName.c_str());

		cameraInfo.inputInfo.inFiledir    = infiledir;


		char filename[100];

		//sprintf(filename ,"%s-%s",cameraInfo.streamName.c_str() ,tempinFilename.c_str() );
		sprintf(filename ,"%s",tempinFilename.c_str() );

		cameraInfo.inputInfo.inFilename = filename;

		camera["inwidth"+ to_string(i)] >>cameraInfo.videoInfo.width;

		camera["inheight"+ to_string(i)] >>cameraInfo.videoInfo.height;

		camera["intype"+ to_string(i)] >> cameraInfo.inputInfo.inputType;
		camera["instartframe"+ to_string(i)] >> cameraInfo.inputInfo.startFrame;

		cameraInfo.currentFrame = cameraInfo.inputInfo.startFrame;

		camera["inendframe"+ to_string(i)] >> cameraInfo.inputInfo.endFrame;


		camera["enableoutput"+ to_string(i)] >> cameraInfo.outputInfo.enableOutput;

		


		
		string tempoutfiledir;

		camera["outfiledir"+ to_string(i)] >> tempoutfiledir;

		char outfiledir[100];

		sprintf(outfiledir ,"%s%s", prefix, tempoutfiledir.c_str());

			
		cameraInfo.outputInfo.outFiledir = outfiledir;


		camera["outfilename"+ to_string(i)] >> cameraInfo.outputInfo.outFilename;

		camera["outtype"+ to_string(i)] >>  cameraInfo.outputInfo.outputType;

		int framerangemode;
		camera["framerangemode"+ to_string(i)] >> framerangemode;

		if(framerangemode)
		{
			cameraInfo.outputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.outputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{

			camera["ostartframe"+ to_string(i)] >> cameraInfo.outputInfo.startFrame;
			camera["oendframe"+ to_string(i)] >> cameraInfo.outputInfo.endFrame;
		}

		displayparam["nuc_displayenable"+ to_string(i)] >> cameraInfo.dispParams.displayOutputInfo.enableOutput;

		//displayparam["nuc_displayoutfiledir"] >> cameraInfo.dispParams.displayOutputInfo.outFiledir;
		
		string tempnuc_displayoutfiledir;
		
		camera["nuc_displayoutfiledir"+ to_string(i)] >> tempnuc_displayoutfiledir;

		camera["startoffset"+ to_string(i)] >>  cameraInfo.startFrameOffset;


		char nuc_displayoutfiledir[100];

		sprintf(nuc_displayoutfiledir ,"%s%s", prefix, tempnuc_displayoutfiledir.c_str());

			
		cameraInfo.dispParams.displayOutputInfo.outFiledir = nuc_displayoutfiledir;


		displayparam["nuc_displayoutfname"+ to_string(i)] >> cameraInfo.dispParams.displayOutputInfo.outFilename;
		displayparam["nuc_displayouttype"+ to_string(i)] >> cameraInfo.dispParams.displayOutputInfo.outputType;
		displayparam["nuc_displayostartframe"+ to_string(i)] >> cameraInfo.dispParams.displayOutputInfo.startFrame;
		displayparam["nuc_displayoendframe"+ to_string(i)] >> cameraInfo.dispParams.displayOutputInfo.endFrame;

		bemparam["bounceSVMModelPath"+ to_string(i)] >> cameraInfo.bemParams.bounceModelPath;
		bemparam["racketSVMModelPath"+ to_string(i)] >> cameraInfo.bemParams.racketHitModelPath;
		bemparam["enableBEM"+ to_string(i)] >> cameraInfo.bemParams.enableBEM;

		odparam["bgstype"+ to_string(i)] >> cameraInfo.odParams.bgsParams.bgsType ;

		odparam["od_enableoutput"+ to_string(i)]>> cameraInfo.odParams.odOutputInfo.enableOutput;
		//odparam["od_outfiledir"] >> cameraInfo.odParams.odOutputInfo.outFiledir;
		
		string tempod_outfiledir;
		
		odparam["od_outfiledir"+ to_string(i)] >> tempod_outfiledir;
		
		char od_outfiledir[100];
		
		sprintf(od_outfiledir ,"%s%s", prefix, tempod_outfiledir.c_str());
		
		cameraInfo.odParams.odOutputInfo.outFiledir = od_outfiledir;

		odparam["od_outfname"+ to_string(i)] >> cameraInfo.odParams.odOutputInfo.outFilename;

		odparam["od_outtype"+ to_string(i)] >>cameraInfo.odParams.odOutputInfo.outputType;

		if(framerangemode)
		{
			cameraInfo.odParams.odOutputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.odParams.odOutputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{
			odparam["od_ostartframe"+ to_string(i)] >> cameraInfo.odParams.odOutputInfo.startFrame;
			odparam["od_oendframe"+ to_string(i)] >>  cameraInfo.odParams.odOutputInfo.endFrame;
		}

		odparam["od_bgsenableoutput"+ to_string(i)] >> cameraInfo.odParams.bgsParams.bgsOutputInfo.enableOutput;
		//odparam["od_outfiledir_bgs"] >> cameraInfo.odParams.bgsParams.bgsOutputInfo.outFiledir;

		string tempod_outfiledir_bgs;
		odparam["od_outfiledir_bgs"+ to_string(i)] >> tempod_outfiledir_bgs;

		char od_outfiledir_bgs[100];
		
		sprintf(od_outfiledir_bgs ,"%s%s", prefix, tempod_outfiledir_bgs.c_str());
		
		cameraInfo.odParams.bgsParams.bgsOutputInfo.outFiledir = od_outfiledir_bgs;

		odparam["od_bgsouttype"+ to_string(i)] >> cameraInfo.odParams.bgsParams.bgsOutputInfo.outputType;

		if(framerangemode)
		{
			cameraInfo.odParams.bgsParams.bgsOutputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.odParams.bgsParams.bgsOutputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{

			odparam["od_bgsstartframe"+ to_string(i)] >> cameraInfo.odParams.bgsParams.bgsOutputInfo.startFrame;
			odparam["od_bgsendframe"+ to_string(i)] >> cameraInfo.odParams.bgsParams.bgsOutputInfo.endFrame;
		}

		ppparam["pose_enablepose"+ to_string(i)] >> cameraInfo.ppParams.enablePose;

		ppparam["pose_infiledir"+ to_string(i)] >> cameraInfo.ppParams.oposeInputInfo.inFiledir;
		ppparam["pose_infilename"+ to_string(i)] >> cameraInfo.ppParams.oposeInputInfo.inFilename;
		ppparam["pose_intype"+ to_string(i)] >> cameraInfo.ppParams.oposeInputInfo.inputType;

		if(framerangemode)
		{
			cameraInfo.ppParams.oposeInputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.ppParams.oposeInputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{
			ppparam["pose_instartframe"+ to_string(i)] >> cameraInfo.ppParams.oposeInputInfo.startFrame;
			ppparam["pose_inendframe"+ to_string(i)] >>  cameraInfo.ppParams.oposeInputInfo.endFrame;
		}

		ppparam["pose_enableoutput"+ to_string(i)]>> cameraInfo.ppParams.oposeOutputInfo.enableOutput;
		//ppparam["pose_outfiledir"] >> cameraInfo.ppParams.oposeOutputInfo.outFiledir;

		string temppose_outfiledir;
	
		ppparam["pose_outfiledir"+ to_string(i)] >> temppose_outfiledir;

		char pose_outfiledir[100];

		sprintf(pose_outfiledir ,"%s%s", prefix, temppose_outfiledir.c_str());

		cameraInfo.ppParams.oposeOutputInfo.outFiledir = pose_outfiledir;

		ppparam["pose_outfname"+ to_string(i)] >> cameraInfo.ppParams.oposeOutputInfo.outFilename;

		ppparam["pose_outtype"+ to_string(i)] >>cameraInfo.ppParams.oposeOutputInfo.outputType;


		if(framerangemode)
		{
			cameraInfo.ppParams.oposeOutputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.ppParams.oposeOutputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{
			ppparam["pose_ostartframe"+ to_string(i)] >> cameraInfo.ppParams.oposeOutputInfo.startFrame;
			ppparam["pose_oendframe"+ to_string(i)] >>  cameraInfo.ppParams.oposeOutputInfo.endFrame;
		}

		ttparam["tt_enableTracker"+ to_string(i)] >> cameraInfo.otParams.ttParams.enableTracker;
		ttparam["tt_runOffset"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOffset;
		ttparam["tt_initConfigPath"+ to_string(i)] >> cameraInfo.otParams.ttParams.initConfigPath;

		ttparam["tt_enableExcelLog"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableExcelDump;
		ttparam["tt_enableTrackInfo"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableTrackInfo;
		ttparam["tt_enablePredictedState"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enablePredState;
		ttparam["tt_enableCorrectedState"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableCorrState;
		ttparam["tt_enableCost"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableCost;
		ttparam["tt_enableRankScore"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableRS;
		ttparam["tt_enableTrackScore"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableTS;
		ttparam["tt_enableCentroids"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableCentroids;
		ttparam["tt_enableBTCentroids"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableBTCentroids;
		ttparam["tt_enablePreCov"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enablePreCov;
		ttparam["tt_enablePostCov"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enablePostCov;
		ttparam["tt_enablePerformance"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enablePerformance;
		ttparam["tt_enableBTData"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableBTData;
		ttparam["tt_enableProfile"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.enableProfile;


		if(framerangemode)
		{
			cameraInfo.otParams.ttParams.excelParams.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.otParams.ttParams.excelParams.endFrame= cameraInfo.inputInfo.endFrame;
		}
		else{
			ttparam["tt_ologStartFrame"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.startFrame;
			ttparam["tt_ologEndFrame"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.endFrame;
		}


		//ttparam["tt_outlogfiledir"] >> cameraInfo.otParams.ttParams.excelParams.outFiledir;

		string temptt_outlogfiledir;

		ttparam["tt_outlogfiledir"+ to_string(i)] >> temptt_outlogfiledir;
		
		char tt_outlogfiledir[100];

		sprintf(tt_outlogfiledir ,"%s%s", prefix, temptt_outlogfiledir.c_str());

		cameraInfo.otParams.ttParams.excelParams.outFiledir = tt_outlogfiledir ;
	
		ttparam["tt_outlogfname"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.outFilename;
		ttparam["tt_startlogTrackIndex"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.startTrackID;
		ttparam["tt_endlogTrackIndex"+ to_string(i)] >> cameraInfo.otParams.ttParams.excelParams.endTrackID;

		ttparam["tt_costNonAssignment"+ to_string(i)] >> cameraInfo.otParams.ttParams.costNonAssignment;
		ttparam["tt_enableoutput"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOutputInfo.enableOutput;
		//ttparam["tt_outfiledir"] >> cameraInfo.otParams.ttParams.trackerOutputInfo.outFiledir;

		string temptt_outfiledir;

		ttparam["tt_outfiledir"] >> temptt_outfiledir;

		char tt_outfiledir[100];
		
		sprintf(tt_outfiledir ,"%s%s", prefix, temptt_outfiledir.c_str());

		cameraInfo.otParams.ttParams.trackerOutputInfo.outFiledir = tt_outfiledir ;

		ttparam["tt_outfname"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOutputInfo.outFilename;
		ttparam["tt_outtype"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOutputInfo.outputType;

		if(framerangemode)
		{
			cameraInfo.otParams.ttParams.trackerOutputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.otParams.ttParams.trackerOutputInfo.endFrame= cameraInfo.inputInfo.endFrame;
		}
		else{
			ttparam["tt_ostartframe"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOutputInfo.startFrame;
			ttparam["tt_oendframe"+ to_string(i)] >> cameraInfo.otParams.ttParams.trackerOutputInfo.endFrame;
		}


		ptparam["pt_enableTracker"+ to_string(i)] >> cameraInfo.otParams.ptParams.enableTracker;
		ptparam["pt_runOffset"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOffset;
		ptparam["pt_initConfigPath"+ to_string(i)] >> cameraInfo.otParams.ptParams.initConfigPath;

		ptparam["pt_enableExcelLog"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableExcelDump;
		ptparam["pt_enableTrackInfo"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableTrackInfo;
		ptparam["pt_enablePredictedState"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enablePredState;
		ptparam["pt_enableCorrectedState"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableCorrState;
		ptparam["pt_enableCost"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableCost;
		ptparam["pt_enableRankScore"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableRS;
		ptparam["pt_enableTrackScore"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableTS;
		ptparam["pt_enableCentroids"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableCentroids;
		ptparam["pt_enableBTCentroids"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableBTCentroids;
		ptparam["pt_enablePreCov"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enablePreCov;
		ptparam["pt_enablePostCov"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enablePostCov;
		ptparam["pt_enablePerformance"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enablePerformance;
		ptparam["pt_enableBTData"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableBTData;
		ptparam["pt_enableProfile"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.enableProfile;


		if(framerangemode)
		{
			cameraInfo.otParams.ptParams.excelParams.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.otParams.ptParams.excelParams.endFrame= cameraInfo.inputInfo.endFrame;
		}
		else{
			ptparam["pt_ologStartFrame"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.startFrame;
			ptparam["pt_ologEndFrame"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.endFrame;
		}

		//ptparam["pt_outlogfiledir"] >> cameraInfo.otParams.ptParams.excelParams.outFiledir;
		
		string temppt_outlogfiledir;
		
		ptparam["pt_outlogfiledir"+ to_string(i)] >> temppt_outlogfiledir;

		char pt_outlogfiledir[100];

		sprintf(pt_outlogfiledir ,"%s%s", prefix, temppt_outlogfiledir.c_str());

		cameraInfo.otParams.ptParams.excelParams.outFiledir = pt_outlogfiledir;


		ptparam["pt_outlogfname"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.outFilename;
		ptparam["pt_startlogTrackIndex"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.startTrackID;
		ptparam["pt_endlogTrackIndex"+ to_string(i)] >> cameraInfo.otParams.ptParams.excelParams.endTrackID;

		ptparam["pt_costNonAssignment"+ to_string(i)] >> cameraInfo.otParams.ptParams.costNonAssignment;
		ptparam["pt_enableoutput"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOutputInfo.enableOutput;
		//ptparam["pt_outfiledir"] >> cameraInfo.otParams.ptParams.trackerOutputInfo.outFiledir;

		string temppt_outfiledir;
		
		ptparam["pt_outfiledir"+ to_string(i)] >> temppt_outfiledir;

		char pt_outfiledir[100];

		sprintf(pt_outfiledir ,"%s%s", prefix, temppt_outfiledir.c_str());

		cameraInfo.otParams.ptParams.trackerOutputInfo.outFiledir =  pt_outfiledir;

		ptparam["pt_outfname"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOutputInfo.outFilename;
		ptparam["pt_outtype"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOutputInfo.outputType;

		if(framerangemode)
		{
			cameraInfo.otParams.ptParams.trackerOutputInfo.startFrame = cameraInfo.inputInfo.startFrame;
			cameraInfo.otParams.ptParams.trackerOutputInfo.endFrame = cameraInfo.inputInfo.endFrame;
		}
		else{
			ptparam["pt_ostartframe"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOutputInfo.startFrame;
			ptparam["pt_oendframe"+ to_string(i)] >> cameraInfo.otParams.ptParams.trackerOutputInfo.endFrame;
		}

		mat_t *matfp;
		matvar_t *matvar_P;
		matvar_t *matvar_Q;
		matvar_t *matvar_R;


		int    start[2]={0,0};
		int    stride[2]={1,1};
		int    edge[2];


		cameraInfo.otParams.ttParams.measurementNoiseCov = cv::Mat::zeros(Size(2,2), CV_64FC1);
		cameraInfo.otParams.ttParams.errorCovPost = cv::Mat::zeros(Size(6,6), CV_64FC1);
		cameraInfo.otParams.ttParams.processNoiseCov = cv::Mat::zeros(Size(6,6), CV_64FC1);


		matfp = Mat_Open(cameraInfo.otParams.ttParams.initConfigPath.c_str(), MAT_ACC_RDONLY );

		if(matfp == nullptr)
		{

			cout<<"Toss Mat Init Config file Open Error \n"  ;
			CV_Assert(0);

		}


		matvar_R = Mat_VarReadInfo(matfp,"Rest1");


		if(matvar_R == nullptr  )
		{

			cout<<"Error in Reading 'R'(measurementNoiseCov) Mat variable \n"  ;
			CV_Assert(0);
		}

		else
		{

			//cout<<"Reading 'R'(measurementNoiseCov) Mat variable \n" <<endl;

			edge[0]=matvar_R->dims[0];
			edge[1]=matvar_R->dims[1];

			edge[0]= 2;
			edge[1]= 2;

			Mat_VarReadData(matfp,matvar_R,cameraInfo.otParams.ttParams.measurementNoiseCov.data,start,stride,edge);


		}

		cameraInfo.otParams.ttParams.measurementNoiseCov *= 10;



		matvar_P = Mat_VarReadInfo(matfp,"P");


		if(matvar_P == nullptr  )
		{
			cout<<"Error in Reading 'P'(errorCovPost) Mat variable \n"  ;
			CV_Assert(0);
		}
		else{

			//cout<<"Reading 'P'(errorCovPost) Mat variable \n" <<endl;

			edge[0]=matvar_P->dims[0];
			edge[1]=matvar_P->dims[1];


			Mat_VarReadData(matfp,matvar_P,(void *) cameraInfo.otParams.ttParams.errorCovPost.data,start,stride,edge);

		}


		matvar_Q = Mat_VarReadInfo(matfp,"Qest1");

		if(matvar_Q == nullptr  )
		{

			cout<<"Error in Reading 'Q'(processNoiseCov) Mat variable \n"  ;
			CV_Assert(0);
		}

		else
		{
			//cout<<"Reading 'Q'(processNoiseCov) Mat variable \n" <<endl;

			edge[0]=matvar_Q->dims[0];
			edge[1]=matvar_Q->dims[1];

			Mat_VarReadData(matfp,matvar_Q,(void *) cameraInfo.otParams.ttParams.processNoiseCov.data,start,stride,edge);

		}

		cameraInfo.otParams.ttParams.processNoiseCov *= 10;

		cameraInfo.otParams.ttParams.measurementNoiseCov.convertTo(cameraInfo.otParams.ttParams.measurementNoiseCov , CV_32FC1);
		cameraInfo.otParams.ttParams.errorCovPost.convertTo(cameraInfo.otParams.ttParams.errorCovPost, CV_32FC1);
		cameraInfo.otParams.ttParams.processNoiseCov.convertTo(cameraInfo.otParams.ttParams.processNoiseCov , CV_32FC1);


		Mat_Close(matfp);

		Mat_VarFree(matvar_P);
		Mat_VarFree(matvar_Q);
		Mat_VarFree(matvar_R);


		cameraInfo.otParams.ptParams.measurementNoiseCov = Mat::zeros(Size(2,2), CV_64FC1);
		cameraInfo.otParams.ptParams.errorCovPost = Mat::zeros(Size(6,6), CV_64FC1);
		cameraInfo.otParams.ptParams.processNoiseCov = Mat::zeros(Size(6,6), CV_64FC1);


		matfp = Mat_Open(cameraInfo.otParams.ptParams.initConfigPath.c_str(), MAT_ACC_RDONLY );

		if(matfp == nullptr)
		{

			cout<<"Proj Mat Init Config file Open Error \n"  ;
			CV_Assert(0);

		}


		matvar_R = Mat_VarReadInfo(matfp,"Rest1");


		if(matvar_R == nullptr  )
		{

			cout<<"Error in Reading 'R'(measurementNoiseCov) Mat variable \n"  ;
			CV_Assert(0);
		}

		else
		{
			//cout<<"Reading 'R'(measurementNoiseCov) Mat variable \n" <<endl;

			edge[0]=matvar_R->dims[0];
			edge[1]=matvar_R->dims[1];

			edge[0]= 2;
			edge[1]= 2;

			Mat_VarReadData(matfp,matvar_R,cameraInfo.otParams.ptParams.measurementNoiseCov.data,start,stride,edge);


		}

		cameraInfo.otParams.ptParams.measurementNoiseCov *= 10;


		matvar_P = Mat_VarReadInfo(matfp,"P");


		if(matvar_P == nullptr  )
		{
			cout<<"Error in Reading 'P'(errorCovPost) Mat variable \n"  ;
			CV_Assert(0);
		}
		else{

			//cout<<"Reading 'P'(errorCovPost) Mat variable \n" <<endl;

			edge[0]=matvar_P->dims[0];
			edge[1]=matvar_P->dims[1];

			Mat_VarReadData(matfp,matvar_P,(void *) cameraInfo.otParams.ptParams.errorCovPost.data,start,stride,edge);

		}


		matvar_Q = Mat_VarReadInfo(matfp,"Qest1");

		if(matvar_Q == nullptr  )
		{

			cout<<"Error in Reading 'Q'(processNoiseCov) Mat variable \n"  ;
			CV_Assert(0);
		}

		else
		{
			//cout<<"Reading 'Q'(processNoiseCov) Mat variable \n" <<endl;

			edge[0]=matvar_Q->dims[0];
			edge[1]=matvar_Q->dims[1];

			Mat_VarReadData(matfp,matvar_Q,(void *) cameraInfo.otParams.ptParams.processNoiseCov.data,start,stride,edge);

		}

		cameraInfo.otParams.ptParams.processNoiseCov *= 10;

		cameraInfo.otParams.ptParams.measurementNoiseCov.convertTo(cameraInfo.otParams.ptParams.measurementNoiseCov , CV_32FC1);
		cameraInfo.otParams.ptParams.errorCovPost.convertTo(cameraInfo.otParams.ptParams.errorCovPost , CV_32FC1);
		cameraInfo.otParams.ptParams.processNoiseCov.convertTo(cameraInfo.otParams.ptParams.processNoiseCov , CV_32FC1);

		Mat_Close(matfp);

		Mat_VarFree(matvar_P);
		Mat_VarFree(matvar_Q);
		Mat_VarFree(matvar_R);


		if( i == m_pCameraInputParams->startCam)
		{
			cameraInfo.currentState = STATE_SERVE;
		}
		else
		{
			cameraInfo.currentState = STATE_PREBOUNCE;
		}


		cameraInfo.currentEvent = EVENT_NONE;


		m_pCameraInputParams->camInfoList.push_back(cameraInfo);

	}



	m_pCameraInputParams->cameraNum = numEnabledCams;
	m_pCameraInputParams->cameraMode = convertbinary2decimal(enableMode , count);


	m_pFSConfig->release();

	
	cout<<"Exiting  aaConfig::parseParams" <<endl;


}


void aaConfig::printParams() const
{

	cout<<"Number of Cameras : "<<m_pCameraInputParams->cameraNum<<endl<<endl;

	for(std::vector<aaCamInfo>::iterator it = m_pCameraInputParams->camInfoList.begin(); it != m_pCameraInputParams->camInfoList.end(); ++it){

		cout<<"Camera Name : " << (*it).name <<endl<<endl;

		cout<<"Run : "<< (*it).enableRun <<endl;
		cout<< "infiledir  : " << (*it).inputInfo.inFiledir << endl ;
		cout<< "infilename : " << (*it).inputInfo.inFilename << endl ;
		cout<< "intype     : " << (*it).inputInfo.inputType << endl ;
		cout<< "instartframe     : " << (*it).inputInfo.startFrame<< endl;
		cout<< "inendframe     : " << (*it).inputInfo.endFrame<< endl;

		cout<< "outfiledir : " << (*it).outputInfo.outFiledir << endl ;
		cout<< "outfilename: " << (*it).outputInfo.outFilename << endl ;
		cout << "camera "  << (*it).name << " " << "outtype    : " << (*it).outputInfo.outputType << endl ;

		cout<<endl<<endl<<"OD Parameters : "<<endl<<endl;
	

		cout<<"live inwidth:"<<(*it).liveParams.inputVideoInfo.width;
		cout<<"live inheight:"<<(*it).liveParams.inputVideoInfo.height;
			


		cout<<"OD BGS type  : "<< 	(*it).odParams.bgsParams.bgsType << endl;

		cout<<"Enable OD Output  : "<< (*it).odParams.odOutputInfo.enableOutput<< endl;
		cout<<"OD outfiledir  : "<< (*it).odParams.odOutputInfo.outFiledir<< endl;
		cout<<"OD outfilename  : "<< (*it).odParams.odOutputInfo.outFilename<< endl;
		cout<<"OD outputType  : "<< (*it).odParams.odOutputInfo.outputType<< endl;
		cout<<"OD startFrame  : "<<(*it).odParams.odOutputInfo.startFrame<< endl;
		cout<<"OD endFrame  : "<<(*it).odParams.odOutputInfo.endFrame<< endl;

		cout<<"BGS outFiledir  : "<< (*it).odParams.bgsParams.bgsOutputInfo.outFiledir<< endl;
		cout<<"BGS startFrame : "<< (*it).odParams.bgsParams.bgsOutputInfo.startFrame<< endl;
		cout<<"BGS endFrame : "<<	(*it).odParams.bgsParams.bgsOutputInfo.endFrame<< endl;

		cout<<"BGS outputType : "<<  (*it).odParams.bgsParams.bgsOutputInfo.outputType << endl;

		cout<<endl<<endl;


		cout<<endl<<endl<<"OT Parameters : "<<endl<<endl;

		cout<<endl<<"TT Parameters : "<<endl<<endl;

		cout<<"Enable Tracker : "<< (*it).otParams.ttParams.enableTracker<<endl;
		cout<<"initConfigPath : "<<  (*it).otParams.ttParams.initConfigPath<<endl;
		cout<<"RunOffset: "<<(*it).otParams.ttParams.trackerOffset<<endl;
		cout<<"Enable Excel Log : "<< (*it).otParams.ttParams.excelParams.enableExcelDump<<endl;
		cout<<"Enable Track Info : "<< (*it).otParams.ttParams.excelParams.enableTrackInfo<<endl;
		cout<<"Enable Pred State : "<< (*it).otParams.ttParams.excelParams.enablePredState<<endl;
		cout<<"Enable Corr State : "<< (*it).otParams.ttParams.excelParams.enableCorrState<<endl;
		cout<<"Enable Cost : "<< (*it).otParams.ttParams.excelParams.enableCost<<endl;
		cout<<"Enable RS : "<< (*it).otParams.ttParams.excelParams.enableRS<<endl;
		cout<<"Enable TS : "<< (*it).otParams.ttParams.excelParams.enableTS<<endl;
		cout<<"Enable Centroids : "<<  (*it).otParams.ttParams.excelParams.enableCentroids<<endl;

		cout<<"Excel Start Frame : "<<  (*it).otParams.ttParams.excelParams.startFrame<<endl;
		cout<<"Excel End Frame : "<<  (*it).otParams.ttParams.excelParams.endFrame<<endl;

		cout<<"Excel outfiledir : "<< (*it).otParams.ttParams.excelParams.outFiledir<<endl;
		cout<<"Excel outfilename : "<< (*it).otParams.ttParams.excelParams.outFilename<<endl;
		cout<<"Excel startTrackID : "<<  (*it).otParams.ttParams.excelParams.startTrackID<<endl;
		cout<<"Excel endTrackID : "<<(*it).otParams.ttParams.excelParams.endTrackID<<endl;

		cout<<"costNonAssignment : "<<(*it).otParams.ttParams.costNonAssignment<<endl;
		cout<<"Enable Output : "<< (*it).otParams.ttParams.trackerOutputInfo.enableOutput<<endl;
		cout<<"Output outFiledir: "<< (*it).otParams.ttParams.trackerOutputInfo.outFiledir<<endl;
		cout<<"Output outFilename: "<< (*it).otParams.ttParams.trackerOutputInfo.outFilename<<endl;
		cout<<"Output outputType: "<< (*it).otParams.ttParams.trackerOutputInfo.outputType<<endl;
		cout<<"Output startFrame: "<<  (*it).otParams.ttParams.trackerOutputInfo.startFrame<<endl;
		cout<<"Output endFrame: "<< (*it).otParams.ttParams.trackerOutputInfo.endFrame<<endl;

		cout<<endl<<"PT Parameters : "<<endl<<endl;

		cout<<"Enable Tracker : "<< (*it).otParams.ptParams.enableTracker<<endl;
		cout<<"initConfigPath : "<< (*it).otParams.ptParams.initConfigPath<<endl;
		cout<<"RunOffset: "<<(*it).otParams.ptParams.trackerOffset<<endl;
		cout<<"Enable Excel Log : "<< (*it).otParams.ptParams.excelParams.enableExcelDump<<endl;
		cout<<"Enable Track Info : "<< (*it).otParams.ptParams.excelParams.enableTrackInfo<<endl;
		cout<<"Enable Pred State : "<< (*it).otParams.ptParams.excelParams.enablePredState<<endl;
		cout<<"Enable Corr State : "<< (*it).otParams.ptParams.excelParams.enableCorrState<<endl;
		cout<<"Enable Cost : "<< (*it).otParams.ptParams.excelParams.enableCost<<endl;
		cout<<"Enable RS : "<< (*it).otParams.ptParams.excelParams.enableRS<<endl;
		cout<<"Enable TS : "<< (*it).otParams.ptParams.excelParams.enableTS<<endl;
		cout<<"Enable Centroids : "<<  (*it).otParams.ptParams.excelParams.enableCentroids<<endl;
		cout<<"Excel Start Frame : "<< (*it).otParams.ptParams.excelParams.startFrame<<endl;
		cout<<"Excel End Frame : "<<  (*it).otParams.ptParams.excelParams.endFrame<<endl;
		cout<<"Excel outfiledir : "<< (*it).otParams.ptParams.excelParams.outFiledir<<endl;
		cout<<"Excel outfilename : "<< (*it).otParams.ptParams.excelParams.outFilename<<endl;
		cout<<"Excel startTrackID : "<< (*it).otParams.ptParams.excelParams.startTrackID<<endl;
		cout<<"Excel endTrackID : "<<(*it).otParams.ptParams.excelParams.endTrackID<<endl;

		cout<<"costNonAssignment : "<< (*it).otParams.ptParams.costNonAssignment<<endl;
		cout<<"Enable Output : "<< (*it).otParams.ptParams.trackerOutputInfo.enableOutput<<endl;
		cout<<"Output outFiledir: "<< (*it).otParams.ptParams.trackerOutputInfo.outFiledir<<endl;
		cout<<"Output outFilename: "<<  (*it).otParams.ptParams.trackerOutputInfo.outFilename<<endl;
		cout<<"Output outputType: "<<  (*it).otParams.ptParams.trackerOutputInfo.outputType<<endl;
		cout<<"Output startFrame: "<<  (*it).otParams.ptParams.trackerOutputInfo.startFrame<<endl;
		cout<<"Output endFrame: "<< (*it).otParams.ptParams.trackerOutputInfo.endFrame<<endl;

		cout<<endl<<endl;


	}

}


cv::FileStorage  * const aaConfig::getpConfig()
{
	return m_pFSConfig;


}

aaCameraInputParams  *  const aaConfig::getpInputParams()
{
	return m_pCameraInputParams;


}


aaConfig::~aaConfig()
{
	//m_pFSConfig->release();

	delete m_pFSConfig;
	m_pFSConfig = nullptr;

	delete m_pCameraInputParams;
	m_pCameraInputParams=nullptr;
}






