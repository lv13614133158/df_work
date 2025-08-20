#ifndef LIVARIANT_HPP
#define LIVARIANT_HPP

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>

class VariantSysInfoClient {
 private:
  std::map<std::string, std::string> kvMap;
  int8_t parseJson(std::string json_path = "/tmp/variant.json");

 public:
  void Init();
  void Deinit();

  std::string GetVinDataIdentifier();                // 0xF190
  std::string GetEcuSerialNumber();                  // 0xF18C
  std::string GetProductSerialNumber();              // 0xFDF6
  std::string GetSystemSupplierIdentifier();         // 0xF18A
  std::string GetSystemSupplierECUHardwareNumber();  // 0xF193
  std::string GetSystemSupplierECUSoftwareNumber();  // 0xF195
  std::string GetOEMHardwareVersion();               // 0xF189
  std::string GetOEMSoftwareVersion();               // 0xF179
};
#endif