/**
 * created on : 2012-07-19
 * author : yanguoyue 
 */

void ImportLibs() {
	
#ifdef MARS_STN
	extern void ExportSTN();
	ExportSTN();
#endif

#ifdef MARS_SMC
	extern void ExportSMC();
	ExportSMC();
#endif

#ifdef MARS_CDN
	extern void ExportCDN();
	ExportCDN();
#endif

#ifdef MARS_MAGICBOX
	extern void ExportMagicbox();
	ExportMagicbox();
#endif

#ifdef MARS_BASEEVENT
	extern void ExportBaseEvent();
	ExportBaseEvent();
#endif

#ifdef MARS_JPEG_STATIC
	extern void ExportMMJpeg();
	ExportMMJpeg();
#endif

#ifdef MARS_SDT
	extern void ExportSDT();
	ExportSDT();
#endif

#ifdef MARS_STATIC_XLOG
	extern void ExportXlog();
	ExportXlog();
#endif	
}
