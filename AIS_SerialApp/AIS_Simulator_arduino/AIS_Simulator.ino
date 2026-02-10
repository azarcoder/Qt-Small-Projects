void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== AIS TEST DATA START ===");
}

void loop()
{
  // ---------------------------------------------------
  // AIS TYPE 1 – POSITION REPORT (CLASS A)
  // ---------------------------------------------------
  // Expected (approx):
  // MMSI: 366053212
  // Lat : 70.67168
  // Lon : 18.31808
  // SOG : 0.0 kn
  // COG : 179.2 deg
  Serial.println("!AIVDM,1,1,,A,15Muq>0P00PD;88MD5MTDww00000,0*1C");
  delay(1500);

  // ---------------------------------------------------
  // AIS TYPE 2 – POSITION REPORT (CLASS A)
  // (Same field layout as Type 1)
  // ---------------------------------------------------
  // Expected:
  // MMSI: 366053213
  // Lat : 70.67170
  // Lon : 18.31810
  // SOG : 0.0 kn
  Serial.println("!AIVDM,1,1,,A,25Muq>0P00PD;88MD5MTDww00000,0*27");
  delay(1500);

  // ---------------------------------------------------
  // AIS TYPE 3 – POSITION REPORT (CLASS A)
  // (Same field layout as Type 1)
  // ---------------------------------------------------
  // Expected:
  // MMSI: 366053214
  // Lat : 70.67172
  // Lon : 18.31812
  Serial.println("!AIVDM,1,1,,A,35Muq>0P00PD;88MD5MTDww00000,0*2E");
  delay(1500);

  // ---------------------------------------------------
  // AIS TYPE 18 – CLASS B POSITION REPORT
  // ---------------------------------------------------
  // Expected:
  // MMSI: 338087561
  // Lat : 59.09683
  // Lon : -156.39316
  // SOG : 0.0 kn
  // COG : 0.0 deg
  Serial.println("!AIVDM,1,1,,B,B52K>Z@005=1K58M33=E@0060000,0*4A");
  delay(3000);
}
