#pragma once
#include <vector>
#include "edsParser.hpp"

namespace mab
{
    /// @brief Enum representing the type of object in the EDS file
    /// @details This enum is used to categorize objects in the EDS file into three types:
    /// MandatoryObjects, OptionalObjects, and ManufacturerObjects.
    /// @note The values correspond to the object types defined in the EDS file.
    enum SectionType_t
    {
        MandatoryObjects    = 0,
        OptionalObjects     = 1,
        ManufacturerObjects = 2,
    };

    /// @brief Structure representing an object in the EDS file
    /// @details Contains fields such as index, subindex, parameter name, storage location, data
    /// type, access type, PDOMapping, default value, and object type.
    /// @note The object type is represented as an enum for better readability.
    struct edsObject
    {
        u32           index           = 0x0000;
        u8            subIndex        = 0x0;
        std::string   ParameterName   = "";
        u8            ObjectType      = 0;
        std::string   StorageLocation = "RAM";
        u32           DataType        = 0x0000;
        std::string   accessType      = "RO";
        bool          PDOMapping      = false;
        u32           defaultValue    = 0;
        SectionType_t sectionType     = ManufacturerObjects;
    };

    std::vector<edsObject> generateObjectDictionary();
}  // namespace mab
// This file is auto-generated from EDS file.
