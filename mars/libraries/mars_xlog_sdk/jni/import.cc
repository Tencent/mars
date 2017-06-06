/**
 * created on : 2012-07-19
 * author : yanguoyue 
 */

void ImportLibs() {
	
#ifdef MARS_STATIC_XLOG
	extern void ExportXlog();
	ExportXlog();
#endif	
}
