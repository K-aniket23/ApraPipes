#include <boost/test/unit_test.hpp>
#include <stdafx.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include "NVRPipeline.h"
#include "PipeLine.h"
#include "ExternalSinkModule.h"
#include "EncodedImageMetadata.h"
#include "FrameContainerQueue.h"
#include "Module.h"
#include "Utils.h"
#include "NVRControlModule.h"
#include "WebCamSource.h"
#include "H264EncoderNVCodec.h"
#include "Mp4WriterSink.h"
#include "CudaMemCopy.h"
#include "CudaStreamSynchronize.h"
#include "H264EncoderNVCodec.h"
#include "ColorConversionXForm.h"
#include "FileReaderModule.h"
#include "ImageViewerModule.h"
#include "KeyboardListener.h"
#include "MultimediaQueueXform.h"
#include "H264Metadata.h"
#include "ValveModule.h"
#include "Mp4ReaderSource.h"
#include "Mp4VideoMetadata.h"
#include "FileWriterModule.h"

BOOST_AUTO_TEST_SUITE(nvrcontrolmodule_tests)


struct CheckThread {
	class SourceModuleProps : public ModuleProps
	{
	public:
		SourceModuleProps() : ModuleProps()
		{};
	};
	class TransformModuleProps : public ModuleProps
	{
	public:
		TransformModuleProps() : ModuleProps()
		{};
	};
	class SinkModuleProps : public ModuleProps
	{
	public:
		SinkModuleProps() : ModuleProps()
		{};
	};

	class SourceModule : public Module
	{
	public:
		SourceModule(SourceModuleProps props) : Module(SOURCE, "sourceModule", props)
		{
		};

	protected:
		bool process() { return false; }
		bool validateOutputPins()
		{
			return true;
		}
		bool validateInputPins()
		{
			return true;
		}
	};
	class TransformModule : public Module
	{
	public:
		TransformModule(TransformModuleProps props) :Module(TRANSFORM, "transformModule", props) {};
	protected:
		bool process() { return false; }
		bool validateOutputPins()
		{
			return true;
		}
		bool validateInputPins()
		{
			return true;
		}
	};
	class SinkModule : public Module
	{
	public:
		SinkModule(SinkModuleProps props) :Module(SINK, "mp4WritersinkModule", props) {};
	protected:
		bool process() { return false; }
		bool validateOutputPins()
		{
			return true;
		}
		bool validateInputPins()
		{
			return true;
		}
	};
};

void key_func(boost::shared_ptr<NVRControlModule>& mControl)
{

	while (true) {
		int k;
		k = getchar();
		if (k == 97)
		{
			BOOST_LOG_TRIVIAL(info) << "Starting Render!!";
			mControl->nvrView(true);
			mControl->step();
		}
		if (k == 100)
		{
			BOOST_LOG_TRIVIAL(info) << "Stopping Render!!";
			mControl->nvrView(false);
			mControl->step();
		}
		if (k == 101)
		{
			boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
			auto now = (boost::posix_time::microsec_clock::universal_time() - time_epoch).total_milliseconds();
			uint64_t seekStartTS = now - 180000;
			uint64_t seekEndTS = now + 120000;
			mControl->nvrExport(seekStartTS, seekEndTS);
			mControl->step();
		}
		if (k == 114)
		{
			BOOST_LOG_TRIVIAL(info) << "Starting Reading from disk!!";
			boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
			auto now = (boost::posix_time::microsec_clock::universal_time() - time_epoch).total_milliseconds();
			uint64_t seekStartTS = now - 5000;
			uint64_t seekEndTS = now;
			mControl->nvrExport(seekStartTS, seekEndTS);
			mControl->step();
		}
		else
		{
			BOOST_LOG_TRIVIAL(info) << "The value pressed is .."<< k;
		}
	}
}

void key_Read_func(boost::shared_ptr<NVRControlModule>& mControl, boost::shared_ptr<Mp4ReaderSource>& mp4Reader)
{

	while (true) {
		int k;
		k = getchar();
		if (k == 97)
		{
			BOOST_LOG_TRIVIAL(info) << "Starting Render!!";
			mControl->nvrView(true);
			mControl->step();
		}
		if (k == 100)
		{
			BOOST_LOG_TRIVIAL(info) << "Stopping Render!!";
			mControl->nvrView(false);
			mControl->step();
		}
		if (k == 101)
		{
			/*uint64_t x, y;
			cout << "Enter start time of Export : ";
			cin >> x;
			cout << "Enter end time of Export : ";
			cin >> y;
			cout << "Start time is " << x << " End time is " << y;*/
			BOOST_LOG_TRIVIAL(info) << "Starting Reading from disk!!";
			boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
			auto now = (boost::posix_time::microsec_clock::universal_time() - time_epoch).total_milliseconds();
			uint64_t seekStartTS = now - 180000;
			uint64_t seekEndTS = now + 120000;
			mControl->nvrExport(seekStartTS, seekEndTS);
			mControl->step();
		}
		if (k == 114)
		{
			BOOST_LOG_TRIVIAL(info) << "Starting Reading from disk!!";
			boost::posix_time::ptime const time_epoch(boost::gregorian::date(1970, 1, 1));
			auto now = (boost::posix_time::microsec_clock::universal_time() - time_epoch).total_milliseconds();
			uint64_t seekStartTS = now + 30000;
			uint64_t seekEndTS = now + 60000;
			mControl->nvrExport(seekStartTS, seekEndTS);
			mControl->step();
			mp4Reader->play(true);
		}
		if (k == 112)
		{
			BOOST_LOG_TRIVIAL(info) << "Stopping Pipeline Input";
		}

		else
		{
			BOOST_LOG_TRIVIAL(info) << "The value pressed is .." << k;
		}
	}
}

BOOST_AUTO_TEST_CASE(basic)
{
	CheckThread f;

	auto m1 = boost::shared_ptr<CheckThread::SourceModule>(new CheckThread::SourceModule(CheckThread::SourceModuleProps()));
	auto metadata1 = framemetadata_sp(new FrameMetadata(FrameMetadata::ENCODED_IMAGE));
	m1->addOutputPin(metadata1);
	auto m2 = boost::shared_ptr<CheckThread::TransformModule>(new CheckThread::TransformModule(CheckThread::TransformModuleProps()));
	m1->setNext(m2);
	auto metadata2 = framemetadata_sp(new FrameMetadata(FrameMetadata::ENCODED_IMAGE));
	m2->addOutputPin(metadata2);
	auto m3 = boost::shared_ptr<CheckThread::TransformModule>(new CheckThread::TransformModule(CheckThread::TransformModuleProps()));
	m2->setNext(m3);
	auto metadata3 = framemetadata_sp(new FrameMetadata(FrameMetadata::ENCODED_IMAGE));
	m3->addOutputPin(metadata3);
	auto m4 = boost::shared_ptr<CheckThread::SinkModule>(new CheckThread::SinkModule(CheckThread::SinkModuleProps()));
	m3->setNext(m4);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	// add all source  modules
	p.appendModule(m1);
	// add control module if any
	p.addControlModule(mControl);
	mControl->enrollModule("source", m1);
	mControl->enrollModule("transform_1", m2);
	mControl->enrollModule("writer", m3);
	mControl->enrollModule("sink", m4);
	// init
	p.init();
	// control init - do inside pipeline init
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(10));
	mControl->nvrView(false);
	// dont need step in run_all_threaded
	mControl->step();

	boost::this_thread::sleep_for(boost::chrono::seconds(10));
	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(checkNVR)
{
	auto nvrPipe = boost::shared_ptr<NVRPipeline>(new NVRPipeline());
	nvrPipe->open();
	//nvrPipe->startRecording();
	nvrPipe->close();
}

BOOST_AUTO_TEST_CASE(NVRTest)
{
	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640;
	auto height = 360;

	// test with 0 - with multiple cameras

	WebCamSourceProps webCamSourceprops(0, 1920, 1080);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);
	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);
	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);
	std::string outFolderPath = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps = Mp4WriterSinkProps(1, 1, 24, outFolderPath);
	mp4WriterSinkProps.logHealth = true;
	mp4WriterSinkProps.logHealthFrequency = 10;
	auto mp4Writer = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps));
	encoder->setNext(mp4Writer);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	p.appendModule(webCam);
	// add control module if any
	p.addControlModule(mControl);
	mControl->enrollModule("source", webCam);
	mControl->enrollModule("colorConversion", colorConvt);
	mControl->enrollModule("cudaCopy", copy);
	mControl->enrollModule("encoder", encoder);
	mControl->enrollModule("Writer-1", mp4Writer);
	// init
	p.init();
	// control init - do inside pipeline init
	mControl->init();
	p.run_all_threaded();
	//mControl->nvrRecord(true);
	// dont need step in run_all_threaded
	mControl->step();

	boost::this_thread::sleep_for(boost::chrono::seconds(240));
	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(NVRView)
{
	WebCamSourceProps webCamSourceprops(0, 1920, 1080);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto multique = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(10000, 5000, true)));
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	webCam->setNext(multique);
	multique->setNext(view);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	p.appendModule(webCam);
	boost::thread inp(key_func, mControl);
	p.addControlModule(mControl);
	mControl->enrollModule("filereader", webCam);
	mControl->enrollModule("viewer", view);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(30));
	p.stop();
	p.term();
	p.wait_for_all();
	inp.join();
}

BOOST_AUTO_TEST_CASE(NVRViewKey)
{
	WebCamSourceProps webCamSourceprops(0, 1920, 1080);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	webCam->setNext(view);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	std::thread inp(key_func, mControl);
	p.appendModule(webCam);
	p.addControlModule(mControl);
	mControl->enrollModule("filereader", webCam);
	mControl->enrollModule("viewer", view);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(100));
	p.stop();
	p.term();
	p.wait_for_all();
	inp.join();
}


BOOST_AUTO_TEST_CASE(NVRFile)
{
	std::string inFolderPath = "./data/Raw_YUV420_640x360";
	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1);
	fileReaderProps.fps = 20;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps)); //
	auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
	auto pinId = fileReader->addOutputPin(metadata);
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	fileReader->setNext(view);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	p.appendModule(fileReader);
	p.addControlModule(mControl);
	mControl->enrollModule("filereader", fileReader);
	mControl->enrollModule("viewer", view);

	p.init();
	mControl->init();
	p.run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(15));
	mControl->nvrView(false);
	mControl->step();
	boost::this_thread::sleep_for(boost::chrono::seconds(10));
	mControl->nvrView(true);
	mControl->step();
	boost::this_thread::sleep_for(boost::chrono::seconds(15));

	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(NVRkey)
{
	std::string inFolderPath = "./data/h264_data";
	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1);
	fileReaderProps.fps = 20;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps)); //
	auto encodedImageMetadata = framemetadata_sp(new H264Metadata(704, 576)); 
	auto pinId = fileReader->addOutputPin(encodedImageMetadata);
	//auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 1, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	fileReader->setNext(mp4Writer_1);
	std::string outFolderPath_2 = "./data/testOutput/mp4_videos/ExportVids/";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 1, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = true;
	mp4WriterSinkProps_2.logHealthFrequency = 10;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	//fileReader->setNext(mp4Writer_2);
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	//std::thread inp(key_func, mControl);
	p.appendModule(fileReader);
	p.addControlModule(mControl);
	mControl->enrollModule("filereader", fileReader);
	mControl->enrollModule("writer", mp4Writer_1);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(10));
	p.stop();
	p.term();
	p.wait_for_all();
	//inp.join();
}

BOOST_AUTO_TEST_CASE(NVR_mmq)
{
	std::string inFolderPath = "./data/h264_data";
	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1);
	fileReaderProps.fps = 20;
	fileReaderProps.readLoop = true;
	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps)); //
	auto encodedImageMetadata = framemetadata_sp(new H264Metadata(704, 576));
	auto pinId = fileReader->addOutputPin(encodedImageMetadata);

	auto multiQueue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(30000, 5000, true)));
	fileReader->setNext(multiQueue);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 1, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	multiQueue->setNext(mp4Writer_1);

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	std::thread inp(key_func, mControl);
	p.appendModule(fileReader);
	p.addControlModule(mControl);
	mControl->enrollModule("filereader", fileReader);
	mControl->enrollModule("multimediaQueue", multiQueue);
	mControl->enrollModule("writer", mp4Writer_1);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(50));
	p.stop();
	p.term();
	p.wait_for_all();
	inp.join();
}

BOOST_AUTO_TEST_CASE(NVR_mmq_view)
{
	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 1920;
	auto height = 1020;


	WebCamSourceProps webCamSourceprops(0, 1920, 1080);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);

	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);

	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);


	auto multiQueue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(30000, 5000, true)));
	encoder->setNext(multiQueue);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 1, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	multiQueue->setNext(mp4Writer_1);

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	std::thread inp(key_func, mControl);
	p.appendModule(webCam);
	p.addControlModule(mControl);
	mControl->enrollModule("webcamera", webCam);
	mControl->enrollModule("multimediaQueue", multiQueue);
	mControl->enrollModule("writer", mp4Writer_1);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(60));
	p.stop();
	p.term();
	p.wait_for_all();
	inp.join();
}

BOOST_AUTO_TEST_CASE(checkNVR2) //Use this for testing pipeline note - Only one mp4Writer is present in this pipeline 
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640;
	auto height = 360;


	WebCamSourceProps webCamSourceprops(0, 1920, 1080);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);
	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);
	H264EncoderNVCodecProps encProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames);
	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(encProps));
	copy->setNext(encoder);

	auto multiQueue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(10000, 5000, true)));
	encoder->setNext(multiQueue);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	multiQueue->setNext(mp4Writer_1);

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));

	PipeLine p("test");
	std::thread inp(key_func, mControl);
	p.appendModule(webCam);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("MultimediaQueue", multiQueue);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(360));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}

BOOST_AUTO_TEST_CASE(checkNVR3) //Use this for testing pipeline note - Mimics the actual pipeline
{
	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640; //1920
	auto height = 360; //1020


	WebCamSourceProps webCamSourceprops(0, 640, 360);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);

	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);

	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	encoder->setNext(mp4Writer_1);

	auto multiQue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(10000, 5000, true)));
	encoder->setNext(multiQue);
	std::string outFolderPath_2 = "./data/testOutput/mp4_videos/ExportVids/";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = true;
	mp4WriterSinkProps_2.logHealthFrequency = 10;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	multiQue->setNext(mp4Writer_2);

	//auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.h264")));
	//multiQue->setNext(fileWriter);

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));
	PipeLine p("test");
	std::thread inp(key_func, mControl);
	p.appendModule(webCam);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("MultimediaQueue", multiQue);
	mControl->enrollModule("Writer-2", mp4Writer_2);

	p.init();
	mControl->init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(360));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}

BOOST_AUTO_TEST_CASE(mp4Read)
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	std::string skipDir = "./data/testOutput/mp4_videos/24bpp";
	std::string startingVideoPath = "./data/testOutput/mp4_videos/24bpp/20221023/0013/1669188939867.mp4";
	std::string outPath = "data/testOutput/outFrames";
	uint64_t seekStartTS = 1669119595641;
	uint64_t seekEndTS = 1669119595641 + 10000;
	boost::filesystem::path file("frame_??????.h264");
	auto frameType = FrameMetadata::FrameType::H264_DATA;
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));

	boost::filesystem::path dir(outPath);

	auto mp4ReaderProps = Mp4ReaderSourceProps(startingVideoPath, false, true);
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
	mp4Reader->addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);

	mp4ReaderProps.skipDir = skipDir;

	boost::filesystem::path full_path = dir / file;
	LOG_INFO << full_path;
	//std::string outFolderPath_2 = "./data/testOutput/testVids";
	//auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	//mp4WriterSinkProps_2.logHealth = true;
	//mp4WriterSinkProps_2.logHealthFrequency = 10;
	//auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	//mp4Reader->setNext(mp4Writer_2);
	auto fileWriterProps = FileWriterModuleProps("./data/testOutput/mp4WriterModuleFrame_ ????.h264");
	auto fileWriter = boost::shared_ptr<FileWriterModule>(new FileWriterModule(fileWriterProps));
	std::vector<std::string> mImagePin;
	mImagePin = mp4Reader->getAllOutputPinsByType(frameType);
	mp4Reader->setNext(fileWriter, mImagePin);
	boost::shared_ptr<PipeLine> p;
	p = boost::shared_ptr<PipeLine>(new PipeLine("test"));
	p->appendModule(mp4Reader);

	if (!p->init())
	{
		throw AIPException(AIP_FATAL, "Engine Pipeline init failed. Check IPEngine Logs for more details.");
	}

	mp4Reader->setProps(mp4ReaderProps);
	mp4Reader->randomSeek(seekStartTS, seekEndTS);

	p->run_all_threaded();

	boost::this_thread::sleep_for(boost::chrono::seconds(10));

	p->stop();
	p->term();
	p->wait_for_all();
	p.reset();
}

BOOST_AUTO_TEST_CASE(mp4ReadView)
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640; //1920
	auto height = 360; //1020

	//WebCam pipeline
	WebCamSourceProps webCamSourceprops(0, 640, 360);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);

	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);

	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	encoder->setNext(mp4Writer_1);

	//Reader pipeline
	//std::string skipDir = "./data/Mp4_videos/h264_video_metadata/";
	std::string startingVideoPath = "./data/Mp4_videos/h264_video/20221010/0012/1668064027062.mp4";
	std::string outPath = "./data/testOutput/mp4_videos/24bpp";
	std::string changedVideoPath = "./data/testOutput/mp4_videos/24bpp/20221023/0017/";
	boost::filesystem::path file("frame_??????.h264");
	auto frameType = FrameMetadata::FrameType::H264_DATA;
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));

	boost::filesystem::path dir(outPath);

	auto mp4ReaderProps = Mp4ReaderSourceProps(startingVideoPath, false, true);
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
	mp4Reader->addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);

	//mp4ReaderProps.skipDir = skipDir;

	boost::filesystem::path full_path = dir / file;
	LOG_INFO << full_path;
	/*std::string outFolderPath_2 = "./data/testOutput/testVids";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = true;
	mp4WriterSinkProps_2.logHealthFrequency = 10;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	mp4Reader->setNext(mp4Writer_2);*/ //fileWriterModuleFrame_????.jpg
	auto fileWriterProps = FileWriterModuleProps("./data/testOutput/mp4WriterModuleFrame_????.h264");
	auto fileWriter = boost::shared_ptr<FileWriterModule>(new FileWriterModule(fileWriterProps));
	std::vector<std::string> mImagePin;
	mImagePin = mp4Reader->getAllOutputPinsByType(frameType);
	mp4Reader->setNext(fileWriter, mImagePin);
	//Pipeline

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));
	PipeLine p("test");
	std::thread inp(key_Read_func,mControl, mp4Reader);
	p.appendModule(webCam);
	p.appendModule(mp4Reader);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("Reader", mp4Reader);
	//mControl->enrollModule("Writer-2", mp4Writer_2);

	p.init();
	mControl->init();
	mp4Reader->play(false);
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(150));
	for (const auto& folder : boost::filesystem::recursive_directory_iterator(boost::filesystem::path("./data/testOutput/mp4_videos/24bpp/20221023/0018/")))
	{
		if (boost::filesystem::is_regular_file(folder))
		{
			boost::filesystem::path p = folder.path();
			changedVideoPath = p.string();
			break;
		}
	}
	Mp4ReaderSourceProps propsChange(changedVideoPath, true);
	mp4Reader->setProps(propsChange);
	boost::this_thread::sleep_for(boost::chrono::seconds(360));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}

BOOST_AUTO_TEST_CASE(mp4ReadWrite)
{
	LoggerProps loggerProps;
	loggerProps.logLevel = boost::log::trivial::severity_level::info;
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	Logger::initLogger(loggerProps);

	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640; //1920
	auto height = 360; //1020

	//WebCam pipeline
	WebCamSourceProps webCamSourceprops(0, 640, 360);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt);

	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);

	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	encoder->setNext(mp4Writer_1);

	//Reader pipeline
	//std::string skipDir = "./data/Mp4_videos/h264_video_metadata/";
	std::string startingVideoPath = "./data/Mp4_videos/h264_video/20221010/0012/1668064027062.mp4";
	std::string outPath = "./data/testOutput/mp4_videos/24bpp";
	std::string changedVideoPath = "./data/testOutput/mp4_videos/24bpp/20221023/0017/";
	boost::filesystem::path file("frame_??????.h264");
	auto frameType = FrameMetadata::FrameType::H264_DATA;
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));

	boost::filesystem::path dir(outPath);

	auto mp4ReaderProps = Mp4ReaderSourceProps(startingVideoPath, false, true);
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
	mp4Reader->addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);

	//mp4ReaderProps.skipDir = skipDir;

	boost::filesystem::path full_path = dir / file;
	LOG_INFO << full_path;
	std::string outFolderPath_2 = "./data/testOutput/testVids";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = true;
	mp4WriterSinkProps_2.logHealthFrequency = 10;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	mp4Reader->setNext(mp4Writer_2); //fileWriterModuleFrame_????.jpg
	//auto fileWriterProps = FileWriterModuleProps("./data/testOutput/mp4WriterModuleFrame_????.h264");
	//auto fileWriter = boost::shared_ptr<FileWriterModule>(new FileWriterModule(fileWriterProps));
	//std::vector<std::string> mImagePin;
	//mImagePin = mp4Reader->getAllOutputPinsByType(frameType);
	//mp4Reader->setNext(fileWriter, mImagePin);
	//Pipeline

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));
	PipeLine p("test");
	std::thread inp(key_Read_func, mControl, mp4Reader);
	p.appendModule(webCam);
	p.appendModule(mp4Reader);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("Reader", mp4Reader);
	mControl->enrollModule("Writer-2", mp4Writer_2);

	p.init();
	mControl->init();
	mp4Reader->play(false);
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(150));
	for (const auto& folder : boost::filesystem::recursive_directory_iterator(boost::filesystem::path("./data/testOutput/mp4_videos/24bpp/20221024/0018/")))
	{
		if (boost::filesystem::is_regular_file(folder))
		{
			boost::filesystem::path p = folder.path();
			changedVideoPath = p.string();
			break;
		}
	}
	Mp4ReaderSourceProps propsChange(changedVideoPath, true);
	mp4Reader->setProps(propsChange);
	boost::this_thread::sleep_for(boost::chrono::seconds(360));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}


BOOST_AUTO_TEST_CASE(checkNVR4) //Use this for testing pipeline note - Mimics the actual pipeline
{
	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640; //1920
	auto height = 360; //1020

	//WebCam
	WebCamSourceProps webCamSourceprops(0, 640, 360);
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	//Color Conversion 

	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR)));
	webCam->setNext(colorConvtView);

	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(ImageViewerModuleProps("NVR-View")));
	colorConvtView->setNext(view);

	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR)));
	webCam->setNext(colorConvt); //WebCam->ColorConversion

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	copy->setNext(encoder);

	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 10;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	encoder->setNext(mp4Writer_1);

	auto multiQue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(MultimediaQueueXformProps(90000, 30000, true)));
	encoder->setNext(multiQue);

	//auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.h264")));
	//multiQue->setNext(fileWriter);

	//Reader pipeline
	//std::string skipDir = "./data/Mp4_videos/h264_video_metadata/";
	std::string startingVideoPath = "./data/Mp4_videos/h264_video/20221010/0012/1668064027062.mp4";
	std::string outPath = "./data/testOutput/mp4_videos/24bpp";
	std::string changedVideoPath = "./data/testOutput/mp4_videos/24bpp/20221023/0017/";
	boost::filesystem::path file("frame_??????.h264");
	auto frameType = FrameMetadata::FrameType::H264_DATA;
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));

	boost::filesystem::path dir(outPath);

	auto mp4ReaderProps = Mp4ReaderSourceProps(startingVideoPath, false, true);
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
	mp4Reader->addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);

	std::string outFolderPath_2 = "./data/testOutput/mp4_videos/ExportVids/";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = true;
	mp4WriterSinkProps_2.logHealthFrequency = 10;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));

	std::string outFolderPath_3 = "./data/testOutput/mp4_videos/ExportVids/";
	auto mp4WriterSinkProps_3 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_3.logHealth = true;
	mp4WriterSinkProps_3.logHealthFrequency = 10;
	auto mp4Writer_3 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));

	multiQue->setNext(mp4Writer_2);


	boost::filesystem::path full_path = dir / file;
	LOG_INFO << full_path;
	mp4Reader->setNext(mp4Writer_3);

	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(NVRControlModuleProps()));
	PipeLine p("test");
	std::thread inp(key_Read_func, mControl, mp4Reader);
	p.appendModule(webCam);
	p.appendModule(mp4Reader);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Reader", mp4Reader);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("MultimediaQueue", multiQue);
	mControl->enrollModule("Writer-2", mp4Writer_2);
	mControl->enrollModule("Writer-3", mp4Writer_3);

	p.init();
	mControl->init();
	mp4Reader->play(false);
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(150));
	for (const auto& folder : boost::filesystem::recursive_directory_iterator(boost::filesystem::path("./data/testOutput/mp4_videos/24bpp/20221030/0012/")))
	{
		if (boost::filesystem::is_regular_file(folder))
		{
			boost::filesystem::path p = folder.path();
			changedVideoPath = p.string();
			break;
		}
	}
	Mp4ReaderSourceProps propsChange(changedVideoPath, true);
	mp4Reader->setProps(propsChange);
	boost::this_thread::sleep_for(boost::chrono::seconds(600));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}

BOOST_AUTO_TEST_CASE(checkNVR5) //Use this for testing pipeline note - Mimics the actual pipeline
{
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	//Logger::initLogger(logprops);

	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = true;
	auto width = 640; 
	auto height = 360; 

	//WebCam
	WebCamSourceProps webCamSourceprops(0, 640, 360);
	webCamSourceprops.logHealth = true;
	webCamSourceprops.logHealthFrequency = 100;
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));
	
	//Color Conversion View
	auto colorProps1 = ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR);
	colorProps1.logHealth =  true;
	colorProps1.logHealthFrequency = 100;
	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(colorProps1));
	webCam->setNext(colorConvtView);

	//ImageViewer
	ImageViewerModuleProps imgViewerProps("NVR-View");
	imgViewerProps.logHealth = true;
	imgViewerProps.logHealthFrequency = 100;
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(imgViewerProps));
	colorConvtView->setNext(view);

	//Color Conversion to encoder
	auto colorProps2 = ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_YUV420PLANAR);
	colorProps2.logHealth = true;
	colorProps2.logHealthFrequency = 100;
	colorProps2.fps = 30;
	auto colorConvt = boost::shared_ptr<ColorConversion>(new ColorConversion(colorProps2));
	webCam->setNext(colorConvt); //WebCam->ColorConversion

	//Cuda Mem Copy
	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copyProps = CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_);
	copyProps.logHealth = true;
	copyProps.logHealthFrequency = 100;
	copyProps.fps = 30;
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(copyProps));
	colorConvt->setNext(copy);

	//H264 Encoder
	auto encoderProps = H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames);
	encoderProps.logHealth = true;
	encoderProps.logHealthFrequency = 100;
	encoderProps.fps = 30;
	auto encoder = boost::shared_ptr<H264EncoderNVCodec>(new H264EncoderNVCodec(encoderProps));
	copy->setNext(encoder);

	auto sinkProps = ExternalSinkModuleProps();
	sinkProps.logHealth = true;
	sinkProps.logHealthFrequency = 100;
	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule(sinkProps));

	//MP4 Writer-1 (24/7 writer)
	std::string outFolderPath_1 = "./data/testOutput/mp4_videos/24bpp/";
	auto mp4WriterSinkProps_1 = Mp4WriterSinkProps(1, 10, 24, outFolderPath_1);
	mp4WriterSinkProps_1.logHealth = true;
	mp4WriterSinkProps_1.logHealthFrequency = 100;
	mp4WriterSinkProps_1.fps = 30;
	auto mp4Writer_1 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_1));
	//encoder->setNext(mp4Writer_1);
	encoder->setNext(sink);

	//MultimediaQueue 
	auto multiProps = MultimediaQueueXformProps(120000, 30000, true);
	multiProps.logHealth = true;
	multiProps.logHealthFrequency = 100;
	multiProps.fps = 30;
	auto multiQue = boost::shared_ptr<MultimediaQueueXform>(new MultimediaQueueXform(multiProps));
	encoder->setNext(multiQue);

	//auto fileWriter = boost::shared_ptr<Module>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/h264images/Raw_YUV420_640x360????.h264")));
	//multiQue->setNext(fileWriter);

	//MP4 Reader [Source]
	std::string startingVideoPath = "./data/Mp4_videos/h264_video/20221010/0012/1668064027062.mp4";
	std::string outPath = "./data/testOutput/mp4_videos/24bpp";
	std::string changedVideoPath = "./data/testOutput/mp4_videos/24bpp/20221023/0011/";                              
	boost::filesystem::path file("frame_??????.h264");
	auto frameType = FrameMetadata::FrameType::H264_DATA;
	auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));
	boost::filesystem::path dir(outPath);
	auto mp4ReaderProps = Mp4ReaderSourceProps(startingVideoPath, false, true);
	mp4ReaderProps.logHealth = true;
	mp4ReaderProps.logHealthFrequency = 100;
	mp4ReaderProps.fps = 30;
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
	mp4Reader->addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);
	
	//MP4 Writer-2 exports
	std::string outFolderPath_2 = "./data/testOutput/mp4_videos/ExportVids/";
	auto mp4WriterSinkProps_2 = Mp4WriterSinkProps(60, 10, 24, outFolderPath_2);
	mp4WriterSinkProps_2.logHealth = false;
	mp4WriterSinkProps_2.logHealthFrequency = 100;
	mp4WriterSinkProps_2.fps = 30;
	auto mp4Writer_2 = boost::shared_ptr<Mp4WriterSink>(new Mp4WriterSink(mp4WriterSinkProps_2));
	multiQue->setNext(mp4Writer_2);
	boost::filesystem::path full_path = dir / file;
	LOG_INFO << full_path;
	//mp4Reader->setNext(mp4Writer_2);
	mp4Reader->setNext(sink);

	//NVR ControlModule                          
	auto controlProps = NVRControlModuleProps();
	controlProps.logHealth = true;
	controlProps.logHealthFrequency = 100;
	controlProps.fps = 30;
	auto mControl = boost::shared_ptr<NVRControlModule>(new NVRControlModule(controlProps));
	Logger::setLogLevel(boost::log::trivial::severity_level::info);


	PipeLine p("test");
	std::thread inp(key_Read_func, mControl, mp4Reader);
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	p.appendModule(webCam);
	p.appendModule(mp4Reader);
	p.addControlModule(mControl);
	mControl->enrollModule("WebCamera", webCam);
	mControl->enrollModule("Reader", mp4Reader);
	mControl->enrollModule("Renderer", view);
	mControl->enrollModule("Writer-1", mp4Writer_1);
	mControl->enrollModule("MultimediaQueue", multiQue);
	mControl->enrollModule("Writer-2", mp4Writer_2);
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	p.init();
	mControl->init();
	mp4Reader->play(false);
	p.run_all_threaded();
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	boost::this_thread::sleep_for(boost::chrono::seconds(150));
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	for (const auto& folder : boost::filesystem::recursive_directory_iterator(boost::filesystem::path("./data/testOutput/mp4_videos/24bpp/20221114/0012/")))
	{
		if (boost::filesystem::is_regular_file(folder))
		{
			boost::filesystem::path p = folder.path();
			changedVideoPath = p.string();
			break;
		}
	}
	Mp4ReaderSourceProps propsChange(changedVideoPath, true);
	mp4Reader->setProps(propsChange);
	boost::this_thread::sleep_for(boost::chrono::seconds(24000));
	p.stop();
	p.term();
	p.wait_for_all();
	BOOST_LOG_TRIVIAL(info) << "The first thread has stopped";
	inp.join();
}

BOOST_AUTO_TEST_CASE(dummyTester)
{
	Logger::setLogLevel(boost::log::trivial::severity_level::info);
	
	//WebCam
	WebCamSourceProps webCamSourceprops(0, 640, 360);
	webCamSourceprops.logHealth = true;
	webCamSourceprops.logHealthFrequency = 100;
	auto webCam = boost::shared_ptr<WebCamSource>(new WebCamSource(webCamSourceprops));

	//Color Conversion View
	auto colorProps1 = ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR);
	colorProps1.logHealth = true;
	colorProps1.logHealthFrequency = 100;
	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(colorProps1));
	//webCam->setNext(colorConvtView);

	//ImageViewer
	ImageViewerModuleProps imgViewerProps("NVR-View");
	imgViewerProps.logHealth = true;
	imgViewerProps.logHealthFrequency = 100;
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(imgViewerProps));
	webCam->setNext(view);

	//External Sink
	auto sinkProps = ExternalSinkModuleProps();
	sinkProps.logHealth = true;
	sinkProps.logHealthFrequency = 50;
	auto sink = boost::shared_ptr<ExternalSinkModule>(new ExternalSinkModule(sinkProps));
	//colorConvtView->setNext(sink);

	PipeLine p("test");
	p.appendModule(webCam);
	p.init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(100000));
	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_CASE(dummyTester2)
{
	Logger::setLogLevel(boost::log::trivial::severity_level::info);

	//FileReader
	std::string inFolderPath = "./data/Raw_YUV420_640x360";
	auto fileReaderProps = FileReaderModuleProps(inFolderPath, 0, -1);
	fileReaderProps.fps = 30;
	fileReaderProps.readLoop = true;
	fileReaderProps.logHealth = true;
	fileReaderProps.logHealthFrequency = 100;
	auto fileReader = boost::shared_ptr<Module>(new FileReaderModule(fileReaderProps)); //
	auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::RGB, CV_8UC3, 0, CV_8U, FrameMetadata::HOST, true));
	//auto metadata = framemetadata_sp(new RawImageMetadata(640, 360, ImageMetadata::ImageType::MONO, CV_8UC1, 0, CV_8U, FrameMetadata::HOST, true));
	auto pinId = fileReader->addOutputPin(metadata);

	//Color Conversion View
	auto colorProps1 = ColorConversionProps(ColorConversionProps::ConversionType::RGB_TO_BGR);
	colorProps1.logHealth = true;

	colorProps1.logHealthFrequency = 100;
	auto colorConvtView = boost::shared_ptr<ColorConversion>(new ColorConversion(colorProps1));
	fileReader->setNext(colorConvtView);

	//ImageViewer
	ImageViewerModuleProps imgViewerProps("NVR-View");
	imgViewerProps.logHealth = true;
	imgViewerProps.logHealthFrequency = 100;
	auto view = boost::shared_ptr<ImageViewerModule>(new ImageViewerModule(imgViewerProps));
	colorConvtView->setNext(view);

	PipeLine p("test");
	p.appendModule(fileReader);
	p.init();
	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(100000));
	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_SUITE_END()