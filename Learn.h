/*****************************************************************************************************/
 //Initialization in main
gAapDaemonController = new CAAPDaemonController();
gAapDaemonController->SetItsCAAPDaemonServer(pAAPDaemonServer);
pAAPDaemonServer->SetItsCAAPDaemonController(gAapDaemonController);

gAapDaemonController->StartWatchdog();
gAapDaemonController->ResolveDSMServices();
gAapDaemonController->ResolveVBSHService();
gAapDaemonController->ResolvePositionService();
gAapDaemonController->ResolveBTAppMalService();
gAapDaemonController->ReadAapDaemonConfigFromXML(APPDAEMON_TYPES_NS::AAPDEAMON_LCF_FILE_PATH);
gAapDaemonController->RegisterGstreamer();
gAapDaemonController->PrepareALSADevice();
gAapDaemonController->StartComponentLauncher();

/*****************************************************************************************************/
/**************************************************************************************************/
											RegisterGstreamer
/**************************************************************************************************/
	   //Create Touch listener object .CAAPTouchListener class implements callbacks related to surface
		itsCAAPTouchListener = new CAAPTouchListener();

		if (itsCAAPTouchListener != NULL)
		{
			itsCAAPTouchListener->setItsCAAPDaemonController(this);
		}
		
	    //"Creating Gstreamer object."
		itsCAAPGstreamer = new CAAPGstreamer();
		{
				//In this constructor creat Msg Q for video data
				int ret = (m_ReadQueue.CreateQ("/AAPDaemonVideoQ", 10, 100));
				ret = m_ReadQueue.OpenQ("/AAPDaemonVideoQ", O_RDONLY);	
		}
		
		if (NULL != itsCAAPGstreamer)
		{
			itsCAAPGstreamer->SetItsCAAPDaemonController(this);
			bool bStatus = itsCAAPGstreamer->InitGstreamerPipeline();
			if (true == bStatus)
			{
				int gstreamerThreadStatus = pthread_create(&m_iGstreamerThreadId, NULL, &startGstreamerThread, this);
				if (APPDAEMON_TYPES_NS::INT_VALUE_ZERO != gstreamerThreadStatus)
				{
					DBG_ERROR(APPCON_COMP_ID, "Failed to create a thread for Gstreamer");
				}
				else
				{
					DBG_ERROR(APPCON_COMP_ID, "Successfully created a thread for Gstreamer");
				}
			}
			else
			{
				DBG_ERROR(APPCON_COMP_ID, "InitGstreamerPipeline status failed...");
			}
			
/**************************************************************************************************/		


/*******************************************************************************************/
const int bufferSize = (APPDAEMON_TYPES_NS::LENGTH_OF_VIDEO_BUFFER)*(APPDAEMON_TYPES_NS::LENGTH_OF_VIDEO_BUFFER);
struct PipelineData
{
	GstElement *appsrc;
	GstElement *h264parse;
	GstElement *decoder;
	GstElement *mSink;
	GstElement *postProc;
} pipelineData;
CMsgQ m_ReadQueue;

 /*********************************************************************************/
					InitGstreamerPipeline
  /*********************************************************************************/
bool bStatus = itsCAAPGstreamer->InitGstreamerPipeline();
{
	gst/video/video-format.h
	//Creates gstreamer pipeline with required elements
	const char* appName = "AndroidAuto Video";//Human readable app name
	unsigned int width = 863;
	unsigned int height = 518;
	g_set_application_name (appName);
	
	checking initialisation of gstreamer.
	GError *err;
	gboolean bGstStatus = gst_init_check(NULL, NULL, &err);
	
	//Gstreamer initialised successfully
	// appsrc-->h264parse-->vaapidecode-->vaapisink
		mPipeline = gst_pipeline_new ("AndroidAuto");
		//Successfully created the pipeline
		
		pipelineData.appsrc  = gst_element_factory_make("appsrc", "videosrc");
		//Successfully created a pipelineData.appsrc
		
		//Do-Timestamp disable and sync false and no live.
		g_object_set(G_OBJECT(pipelineData.appsrc), "stream-type", GST_APP_STREAM_TYPE_STREAM, "do-timestamp", true, "is-live", true, NULL);
	
		pipelineData.h264parse = gst_element_factory_make("h264parse", "parser");
		//Created a h264parse successfully
		
		g_object_set(G_OBJECT(pipelineData.h264parse), "no-wait-next-nal", true,  NULL);
		
		pipelineData.postProc = gst_element_factory_make("vaapipostproc", "postProc");
		//Successfully created  vaapipostproc
		g_object_set(G_OBJECT(pipelineData.postProc), "format", GST_VIDEO_FORMAT_NV12,  NULL);
		
		gDaemonController->GetScreenType()
		width = APPDAEMON_TYPES_NS::SCALED_WIDTH_HD_800_STREAM;
		height = APPDAEMON_TYPES_NS::SCALED_HEIGHT_HD_800_STREAM;
		
		g_object_set(G_OBJECT(pipelineData.postProc), "width", width, "height", height, NULL);
		
		pipelineData.decoder = gst_element_factory_make("vaapidecode", "vaapi");
		//Successfully created  vaapidecoder
		pipelineData.mSink = gst_element_factory_make("vaapisink", "videoSink");
		//Successfully created  vaapisink
		g_object_set(G_OBJECT(pipelineData.mSink), "sync", false, NULL);
		
		gst_bin_add_many (GST_BIN(mPipeline), pipelineData.appsrc, pipelineData.h264parse, pipelineData.decoder, pipelineData.postProc, pipelineData.mSink, NULL);
	
			if (!gst_element_link_many (pipelineData.appsrc, pipelineData.h264parse, pipelineData.decoder, NULL))
			{
				DBG_ERROR(APPCON_COMP_ID, "Failed to link appsrc, h264parse and decode");
				gst_object_unref (GST_OBJECT (mPipeline));
				mPipeline = NULL;
				bGstStatus = false;
			}
			else
			{
				DBG_INFO(APPCON_COMP_ID, "Appsrc, h264parse and decode linked successfully.");
			}
			
			if (!gst_element_link_many (pipelineData.decoder, pipelineData.postProc, pipelineData.mSink, NULL))
			{
				DBG_ERROR(APPCON_COMP_ID, "Failed to link vaapidecode, vaapispostproc and vaapisink");
				gst_object_unref (GST_OBJECT (mPipeline));
				mPipeline = NULL;
				bGstStatus = false;
			}
			else
			{
				DBG_INFO(APPCON_COMP_ID, "vaapidecode and vaapisink linked successfully");
			}
			
			//Successfully created and linked the required pipeline now connecting to the signals
			
			g_signal_connect(pipelineData.appsrc, "need-data", G_CALLBACK(gal_need_data), NULL);
			g_signal_connect (pipelineData.appsrc, "enough-data", G_CALLBACK (stop_feed), NULL);
			m_bIsActive = true;
}
/*******************************************************************************************/
/*******************************************************************************************/

/*******************************************************************************************/
								startGstreamerThread
/*******************************************************************************************/
		CAAPDaemonController *pController = reinterpret_cast<CAAPDaemonController*>(arg);
		pController->itsCAAPGstreamer->Gstreamer_InternalRun();
		{
				GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(mPipeline));
				GstMessage *msg = NULL;
				while (m_bIsActive)
				{
					// get all the messages that are currently queued in the bus
					do
					{
						msg = gst_bus_pop(bus);
						if ( NULL == msg)
						{
							break; // no mesages at the moment, will try again later
						}
						// take care about bus messages (currently only a eos)
						onBusMessage(bus, msg, this);
						gst_message_unref(msg);
					} while ((NULL != msg ) && (true == m_bIsActive));
					
					if (m_bIsActive) // avoid unnecessary sleep when we are no longer running
					{
						usleep(APPDAEMON_TYPES_NS::GSTREAMER_THREAD_SLEEP);
					}
				}
				gst_element_set_state(GST_ELEMENT(mPipeline), GST_STATE_NULL);
				gst_object_unref(bus);
				gst_object_unref(GST_OBJECT(mPipeline));
				mPipeline = NULL;
		}
/*******************************************************************************************/




/*******************************************************************************************************************/
										PrepareALSADevice
/*******************************************************************************************************************/
CAAPDaemonController::ReadAapDaemonConfigFromXML
{
	itsCFileManager = new CFileManager();
	CFileManager::ReadAapDaemonConfigFile
	CFileManager::ProcessNode
	CFileManager::ExtractNodeValues
	{
		tAAPDeamonConfigParams.vctAudioParams.push_back(tAudio);
	}
}

//"Preparing ALSA Device for Source/Sink[AudioFocusState]"
	//ALSA device for Media
	for (std::vector<APPDAEMON_TYPES_NS::tAudio_Params>::iterator audioItr = m_tAapDaemonParameters.vctAudioParams.begin(); audioItr != m_tAapDaemonParameters.vctAudioParams.end(); audioItr++)
	{
		//uiNumAudioChannels = 2U;	//stereo channel
		if (strncmp(audioItr->strAlsaDeviceName.c_str(), APPDAEMON_TYPES_NS::ALSA_DEV_MEDIA, audioItr->strAlsaDeviceName.length()) == APPDAEMON_TYPES_NS::INTEGER_INIT_ZERO)
		{
			DBG_INFO(APPCON_COMP_ID, "ALSA Device for MEDIA[1/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			iRetStatus = ConfigureALSADevice(audioItr->strAlsaDeviceName, audioItr->uiSamplingRate, audioItr->uiNumberOfChannels, true, APPDAEMON_TYPES_NS::eALSAForMedia);
			m_eAlsaHandle = APPDAEMON_TYPES_NS::eALSAForMedia;
			if(APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
			{
				DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice [SUCCESS]: for MEDIA[1/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
			else
			{
				DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice [FAILED]: for MEDIA[1/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
		}
		else if (strncmp(audioItr->strAlsaDeviceName.c_str(), APPDAEMON_TYPES_NS::ALSA_DEV_GUIDANCE, audioItr->strAlsaDeviceName.length()) == APPDAEMON_TYPES_NS::INTEGER_INIT_ZERO)
		{
			DBG_INFO(APPCON_COMP_ID, "ALSA Device for GUIDANCE[2/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			iRetStatus = ConfigureALSADevice(audioItr->strAlsaDeviceName, audioItr->uiSamplingRate, audioItr->uiNumberOfChannels,true, APPDAEMON_TYPES_NS::eALSAForGuidance);
			if(APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
			{
				DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice [SUCCESS]: for GUIDANCE[2/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
			else
			{
				DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice [FAILED]: for GUIDANCE[2/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
		}
		else if (strncmp(audioItr->strAlsaDeviceName.c_str(), APPDAEMON_TYPES_NS::ALSA_DEV_MICROPHONE, audioItr->strAlsaDeviceName.length()) == APPDAEMON_TYPES_NS::INTEGER_INIT_ZERO)
		{
			//ALSA device for Microphone.
			iRetStatus = ConfigureALSADevice(audioItr->strAlsaDeviceName, audioItr->uiSamplingRate, audioItr->uiNumberOfChannels, false, APPDAEMON_TYPES_NS::eALSAForMIC);
			if (APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
			{
				DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice [SUCCESS]: for MICROPHONE[2/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
			else
			{
				DBG_ERROR(APPCON_COMP_ID, "ConfigureALSADevice [FAILURE]: for MICROPHONE[2/2] = %s |Sampling Rate = %d |Number of Audio Channels = %d", audioItr->strAlsaDeviceName.c_str(), audioItr->uiSamplingRate, audioItr->uiNumberOfChannels);
			}
		}
		else
		{

		}
	}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
												ConfigureALSADevice
/*******************************************************************************************************************/
1]
//Device to be used for playback
DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice for PLAYBACK");
iRetStatus = snd_pcm_open(&l_tALSADevHandle, strALSADeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

//Device to be used for MIC
DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice for MIC");
iRetStatus = snd_pcm_open(&l_tALSADevHandle, strALSADeviceName.c_str(), SND_PCM_STREAM_CAPTURE, APPDAEMON_TYPES_NS::INT_VALUE_ZERO);

2]//Device opened successfully
//Allocate a hardware parameters object.
iRetStatus = snd_pcm_hw_params_malloc(&params);

3]
//Fill it in with default values.
iRetStatus = snd_pcm_hw_params_any(l_tALSADevHandle, params);	

4]
//Set the desired hardware parameters.
//Interleaved mode
iRetStatus = snd_pcm_hw_params_set_access(l_tALSADevHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

5]
//Signed 16-bit little-endian format
iRetStatus = snd_pcm_hw_params_set_format(l_tALSADevHandle, params, SND_PCM_FORMAT_S16_LE);

6]
//snd_pcm_hw_params_set_format. Now snd_pcm_hw_params_set_channels");
iRetStatus = snd_pcm_hw_params_set_channels(l_tALSADevHandle, params, uiNumAudioChannels);

7]							
//######################################### Sampling rate #####################################
iRetStatus = snd_pcm_hw_params_set_rate_near(l_tALSADevHandle, params, &uiSamplingRate, &dir);

8]
//_SUCCESS.snd_pcm_hw_params_set_rate_near. Now snd_pcm_hw_params_set_period_size_near uiSamplingRate = %d ",uiSamplingRate);
iRetStatus = snd_pcm_hw_params_set_period_size_near(l_tALSADevHandle, params, &periodsize, &dir);

9]		
//_SUCCESS.snd_pcm_hw_params_set_period_size_near.Now snd_pcm_hw_params_set_buffer_size_near");
iRetStatus = snd_pcm_hw_params_set_buffer_size_near(l_tALSADevHandle, params, &buffersize);	

10]
// Write the parameters to the driver
//SUCCESS.snd_pcm_hw_params_set_rate_near.Now writing configuration to the device");
iRetStatus = snd_pcm_hw_params(l_tALSADevHandle, params);	

11]
//_SUCCESS.snd_pcm_hw_params. State of Stream = [%d]", snd_pcm_state(l_tALSADevHandle));
iRetStatus = snd_pcm_prepare (l_tALSADevHandle);	
	
/*******************************************************************************************************************/
/*******************************************************************************************************************/
									ConfigureALSADevice
									int CAAPDaemonController::ConfigureALSADevice(std::string strALSADeviceName,
									unsigned int uiSamplingRate,unsigned int  uiNumAudioChannels,bool bIsDeviceSink, 
									APPDAEMON_TYPES_NS::eALSADeviceHandleType eALSADevHandle )
/*******************************************************************************************************************/
	int iConfigureSuccess = APPDAEMON_TYPES_NS::INT_VALUE_ZERO;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_sw_params_malloc(&swparams);
	if(false == GetALSADevReadyStatus(eALSADevHandle)) //device is not already configured and not ready
	{
		int dir = APPDAEMON_TYPES_NS::INT_VALUE_ZERO;
		snd_pcm_hw_params_t *params;
		int iRetStatus = APPDAEMON_TYPES_NS::INT_VALUE_ZERO;

		snd_pcm_uframes_t buffersize = uiSamplingRate/APPDAEMON_TYPES_NS::VALUE_TWO;

		snd_pcm_uframes_t periodsize = buffersize/APPDAEMON_TYPES_NS::VALUE_FOUR;

		DBG_INFO(APPCON_COMP_ID, "Configuring ALSA Device = %s |Sampling Rate = %d |Number of Audio Channels = %d", strALSADeviceName.c_str(), uiSamplingRate, uiNumAudioChannels);
		printf("Configuring ALSA Device = %s |Sampling Rate = %d |Number of Audio Channels = %d\n", strALSADeviceName.c_str(), uiSamplingRate, uiNumAudioChannels);

		snd_pcm_t * l_tALSADevHandle = GetALSADeviceHandle(eALSADevHandle);
		//######################################### Opening Device #####################################
		if(true == bIsDeviceSink)
		{
			//Device to be used for playback
			DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice for PLAYBACK");
			iRetStatus = snd_pcm_open(&l_tALSADevHandle, strALSADeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
		}
		else
		{
			//Device to be used for MIC
			DBG_INFO(APPCON_COMP_ID, "ConfigureALSADevice for MIC");
			iRetStatus = snd_pcm_open(&l_tALSADevHandle, strALSADeviceName.c_str(), SND_PCM_STREAM_CAPTURE, APPDAEMON_TYPES_NS::INT_VALUE_ZERO);
		}

		if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
		{
			//Device opened successfully
			DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:1]_SUCCESS.Device opened successfully.Now configuring the device");
			//Allocate a hardware parameters object.

			iRetStatus = snd_pcm_hw_params_malloc(&params);

			if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
			{
				DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:2]_SUCCESS.snd_pcm_hw_params_malloc. Now snd_pcm_hw_params_any");
				//Fill it in with default values.
				iRetStatus = snd_pcm_hw_params_any(l_tALSADevHandle, params);

				if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
				{
					DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:3]_SUCCESS.snd_pcm_hw_params_any. Now snd_pcm_hw_params_set_access");
					//Set the desired hardware parameters.
					//Interleaved mode
					iRetStatus = snd_pcm_hw_params_set_access(l_tALSADevHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
					if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
					{
						DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:4]_SUCCESS.snd_pcm_hw_params_set_access. Now snd_pcm_hw_params_set_format");
						//Signed 16-bit little-endian format
						iRetStatus = snd_pcm_hw_params_set_format(l_tALSADevHandle, params, SND_PCM_FORMAT_S16_LE);
						//######################################### audio channels #####################################
						if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
						{
							DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:5]_SUCCESS.snd_pcm_hw_params_set_format. Now snd_pcm_hw_params_set_channels");
							iRetStatus = snd_pcm_hw_params_set_channels(l_tALSADevHandle, params, uiNumAudioChannels);
							if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
							{
								DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:6]_SUCCESS.snd_pcm_hw_params_set_channels. Now snd_pcm_hw_params_set_rate_near");
								//######################################### Sampling rate #####################################
								iRetStatus = snd_pcm_hw_params_set_rate_near(l_tALSADevHandle, params, &uiSamplingRate, &dir);
								if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
								{
									DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:7]_SUCCESS.snd_pcm_hw_params_set_rate_near. Now snd_pcm_hw_params_set_period_size_near uiSamplingRate = %d ",uiSamplingRate);

									iRetStatus = snd_pcm_hw_params_set_period_size_near(l_tALSADevHandle, params, &periodsize, &dir);

									if ( APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
									{
										DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:8]_SUCCESS.snd_pcm_hw_params_set_period_size_near.Now snd_pcm_hw_params_set_buffer_size_near");
										iRetStatus = snd_pcm_hw_params_set_buffer_size_near(l_tALSADevHandle, params, &buffersize);
										if (APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)

										{
											// Write the parameters to the driver
											DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:9]_SUCCESS.snd_pcm_hw_params_set_rate_near.Now writing configuration to the device");
											errno = APPDAEMON_TYPES_NS::INT_VALUE_ZERO;
											iRetStatus = snd_pcm_hw_params(l_tALSADevHandle, params);
											if (APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
											{
												DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:10]_SUCCESS.snd_pcm_hw_params. State of Stream = [%d]", snd_pcm_state(l_tALSADevHandle));

												iRetStatus = snd_pcm_prepare (l_tALSADevHandle);
												if (APPDAEMON_TYPES_NS::INT_VALUE_ZERO == iRetStatus)
												{
													DBG_INFO(APPCON_COMP_ID, "[CONFIGURE_ALSA:11]_SUCCESS.snd_pcm_prepare.State of Stream = [%d]", snd_pcm_state(l_tALSADevHandle));
													iConfigureSuccess = APPDAEMON_TYPES_NS::INT_VALUE_ZERO;

												}
												else
												{
													DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:11]_FAILED:Unable to snd_pcm_prepare:[Returned:(%d)=(%s)]::[errno:(%d)=(%s)]",iRetStatus, snd_strerror(iRetStatus),errno,strerror(errno));
													iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
												}
											}
											else
											{
												DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:10]_FAILED:Unable to snd_pcm_hw_params: [Returned:(%d)=(%s)]::[errno:(%d)=(%s)]",iRetStatus, snd_strerror(iRetStatus),errno,strerror(errno));
												iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
											}
										}
										else
										{
											DBG_ERROR(APPCON_COMP_ID,"[CONFIGURE_ALSA:9]_FAILED:Unable to snd_pcm_hw_params_set_buffer_size_near: (%s)", snd_strerror(iRetStatus));
											iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
										}
									}
									else
									{
										DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:8]_FAILED:Unable to snd_pcm_hw_params_set_period_size_near: (%s)", snd_strerror(iRetStatus));
										iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
									}
								}
								else
								{
									DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:7]_FAILED:Unable to snd_pcm_hw_params_set_rate_near: (%s)", snd_strerror(iRetStatus));
									iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
								}
							}
							else
							{
								DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:6]_FAILED:Unable to snd_pcm_hw_params_set_channels: (%s)", snd_strerror(iRetStatus));
								iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
							}
						}
						else
						{
							DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:5]_FAILED:Unable to snd_pcm_hw_params_set_format: (%s)", snd_strerror(iRetStatus));
							iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
						}
					}
					else
					{
						DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:4]_FAILED:Unable to snd_pcm_hw_params_set_access: (%s)", snd_strerror(iRetStatus));
						iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
					}
				}
				else
				{
					DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:3]_FAILED:Unable to snd_pcm_hw_params_any: (%s)", snd_strerror(iRetStatus));
					iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
				}
			}
			else
			{
				DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:2]_FAILED:Unable to snd_pcm_hw_params_alloca: (%s)", snd_strerror(iRetStatus));
				iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
			}
			snd_pcm_hw_params_free(params);
		}
		else
		{
			DBG_ERROR(APPCON_COMP_ID, "[CONFIGURE_ALSA:1]_FAILED:Unable to open pcm device: (%s)", snd_strerror(iRetStatus));
			iConfigureSuccess = APPDAEMON_TYPES_NS::NEVAGATIVE_VALUE_ONE;
		}

		if (APPDAEMON_TYPES_NS::INTEGER_INIT_ZERO == iConfigureSuccess)
		{
			DBG_INFO(APPCON_COMP_ID, "ALSA Device configured and handle value = [%u]",l_tALSADevHandle);
			SetSWParamsForALSADev(l_tALSADevHandle, swparams, buffersize, periodsize);
			SetALSADeviceHandle(eALSADevHandle, l_tALSADevHandle);
			SetALSADevReadyStatus(eALSADevHandle, true);
		}
		else
		{
			SetALSADevReadyStatus(eALSADevHandle, false);
		}
	}
	else
	{
		//device is already configured and ready
	}

	return iConfigureSuccess;
/*******************************************************************************************************************/



InitializeCommunication 
connecting with all corba clients
eStatus = itsCAAPDaemonClient->ConnectToAAPDaemon(iArgc,pArgList);


evAAHMI_CAVStartAAPSessionReq 
	CAV_DEVICE_TYPE_E l_eDeviceType = params->eDeviceType; 
	
	// Get Android Device Information from PDM
		CAV_ANDROID_SESSION_INFO_T l_tDeviceDetails = {};
		PDM_GetCAVAndroidDevSessionInfo(&l_tDeviceDetails);
		// Storing AOAP Device info in CAADevice object for reference
			itsCAADevice->SetDeviceInfo(l_tDeviceDetails.tAndroidDevInfo); 

	m_bIsDeviceDisplayFlag = l_tDeviceDetails.bAutoDisplay;   
		
		// Storing First Run flag info for the device in our data member for reference  
		m_bIsFirstRunFlag = l_tDeviceDetails.bFirstRunFlag;

// Requesting AAP Daemon to start an AAP session with the requested device
			itsCAAPDaemonClient->AAMWStartAAPSession(l_strSocketAddress);
			
LifeCycle_StartAapSession

1]RegisterGalServices
{
RegisterGalReceiver
{
itsCAAPGalReceiver = new CAAPGalReceiver();
itsCAAPControllerCallbacks = new CAAPControllerCallbacks();

itsCAAPControllerCallbacks->SetItsCAAPDaemonController(this);
bGalInitStatus = itsCAAPGalReceiver->InitGal(itsCAAPControllerCallbacks);			
}
}
2]InitialiseGalReceiver
bIsGalInitilised = itsCAAPGalReceiver->InitialiseGALReceiver(m_tCLientInfo);
3]//Initialise all the required services
itsCAAPGstreamer->StartPlaying();
InitialiseAudioSinkMedia();
		InitialiseAudioSinkNav();
		InitialiseVideoSinkService();
		InitialiseAudioSource();
		InitialiseBluetoothEndPoint();
		InitialiseInputSourceService();
		InitialiseSensorSource();

itsCAAPVideoSinkCallbacks->setItsCAAPGstreamer(itsCAAPGstreamer);


//Start the GAL receiver library and wait for 15 sec
itsCAAPGalReceiver->StartGal();
InitializeReportDataTimers();

itsCAAPPositioningClient->GetCountryCode();
itsCAAPPositioningClient->UpdateReportPositionDataFlag();

socket
connect
			

evAAHMI_GoOnView 
{
	m_bIsAAPSessionActive
	m_bIsVideoSinkSetupReceived
	
	check l_eContextId E_CONTEXT_HOME_NAV_TO_DEFAULT_PAGE
	check m_bIsVideoUnsolicited
	// Requesting the device to start the Projection
	itsCAAPDaemonClient->AAMWSetVideoFocus(eProjected,m_bIsVideoUnsolicited);
}	
			
