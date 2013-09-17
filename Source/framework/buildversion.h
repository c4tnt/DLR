// This should never be hand edited, AlienBrain will update this when the
// release new doom .exe is executed from within the AlienBrain client
// There are no consequences for it not being updated, it just lets us
// track an exact build number that matches the source control history
#if (API_VERS >= 4)
const int BUILD_NUMBER = 1304;
#elif (API_VERS >= 3)
const int BUILD_NUMBER = 1302;
#else
const int BUILD_NUMBER = 1282;
#endif