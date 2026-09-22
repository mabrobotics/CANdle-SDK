#ifndef MD_OBJECTS_HPP
#define MD_OBJECTS_HPP

#include <array>
#include <cctype>
#include <iomanip>
#include <mutex>
#include <set>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "edsEntry.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @namespace md_objects
    /// @brief Every object of the MD object dictionary the SDK depends on.
    ///
    /// This is the single place to update when a new .eds revision ships. An object is described
    /// by the address it has in the newest .eds plus the ParameterName it carried in that and in
    /// every earlier revision, because neither of the two is stable on its own:
    ///
    ///  - names get reworded between revisions ("Run Can Reinit" became "Reinitialise CAN",
    ///    "Modes Of Operation" became "Modes of Operation")
    ///  - manufacturer indices get reorganized (the command record moved from 0x2003 to 0x2023,
    ///    and 0x2003 now holds a completely different object)
    ///
    /// resolveObject() therefore addresses by index and confirms the identity by name, so neither
    /// a rename nor a renumbering can make it act on the wrong object.
    namespace md_objects
    {
        /// @brief Address of an object plus the names it is known to have carried
        struct ObjectRef
        {
            /// @brief index the object has in the newest .eds revision
            u16 index = 0;
            /// @brief subindex of the object, empty for objects addressed by index alone
            std::optional<u8> subIndex;
            /// @brief name used to report the object, never used to locate it
            std::string_view label;
            /// @brief ParameterNames the object carried, newest revision first, empty entries
            /// are ignored. For a sub-entry these are the names of the sub-entry itself
            std::array<std::string_view, 3> aliases;
            /// @brief ParameterNames of the record holding this sub-entry, needed when the name
            /// of the sub-entry repeats across records ("Kp" exists in every PID record) or when
            /// the subindex was reshuffled between .eds revisions
            std::array<std::string_view, 2> parentAliases;
        };

        /// @brief Compare two ParameterNames, ignoring case
        ///
        /// Capitalisation is not stable between .eds revisions, md_1.2 recapitalized over a
        /// hundred names inherited from MDv1.0.0 ("Pole pairs" became "Pole Pairs"), so names
        /// are always compared case insensitively
        inline bool namesMatch(std::string_view lhs, std::string_view rhs)
        {
            if (lhs.size() != rhs.size())
                return false;
            for (size_t i = 0; i < lhs.size(); i++)
            {
                if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
                    std::tolower(static_cast<unsigned char>(rhs[i])))
                    return false;
            }
            return true;
        }

        /// @brief Find an entry of the dictionary carrying one of the given names
        /// @param od object dictionary parsed from the .eds file
        /// @param names names to look for, empty entries are ignored
        /// @return entry carrying one of the names, nullptr when none does
        inline EDSEntry* findByAnyName(EDSObjectDictionary&                   od,
                                       const std::array<std::string_view, 2>& names)
        {
            for (const auto& name : names)
            {
                if (name.empty())
                    continue;
                for (auto& entry : od)
                {
                    if (namesMatch(entry.second.getEntryMetaData().parameterName, name))
                        return &entry.second;
                }
            }
            return nullptr;
        }

        /// @brief Find a sub-entry of a record carrying one of the given names
        /// @param record record to search through
        /// @param names names to look for, empty entries are ignored
        /// @return sub-entry carrying one of the names, nullptr when none does
        inline EDSEntry* findSubByAnyName(EDSEntry&                              record,
                                          const std::array<std::string_view, 3>& names)
        {
            for (const auto& name : names)
            {
                if (name.empty())
                    continue;
                for (auto& subObject : record)
                {
                    if (namesMatch(subObject.second->getEntryMetaData().parameterName, name))
                        return subObject.second.get();
                }
            }
            return nullptr;
        }

        // ---- CiA 402 objects, standardized indices ----------------------------------------
        inline constexpr ObjectRef MODES_OF_OPERATION{
            .index   = 0x6060,
            .label   = "modes of operation",
            .aliases = {"Modes of Operation", "Modes Of Operation"}};

        inline constexpr ObjectRef MAX_CURRENT{
            .index = 0x6073, .label = "max current", .aliases = {"Max Current"}};

        inline constexpr ObjectRef MOTOR_RATED_CURRENT{
            .index = 0x6075, .label = "motor rated current", .aliases = {"Motor Rated Current"}};

        // ---- MAB manufacturer objects ------------------------------------------------------
        inline constexpr ObjectRef TORQUE_BANDWIDTH{.index    = 0x2000,
                                                    .subIndex = u8{0x05},
                                                    .label    = "torque bandwidth",
                                                    .aliases  = {"Torque Bandwidth"}};

        inline constexpr ObjectRef POSITION_PID_CONTROLLER{
            .index = 0x2010, .label = "position PID controller", .aliases = {
                "Position PID Controller"}};

        inline constexpr ObjectRef VELOCITY_PID_CONTROLLER{
            .index = 0x2011, .label = "velocity PID controller", .aliases = {
                "Velocity PID Controller"}};

        inline constexpr ObjectRef IMPEDANCE_PD_CONTROLLER{
            .index = 0x2012, .label = "impedance PD controller", .aliases = {
                "Impedance PD Controller"}};

        // Status record, 0x2022 in md_1.2, was 0x2004 "System Status" in MDv1.0.0
        inline constexpr ObjectRef MAIN_ENCODER_STATUS{.index    = 0x2022,
                                                       .subIndex = u8{0x02},
                                                       .label    = "main encoder status",
                                                       .aliases  = {"Main Encoder Status"}};

        inline constexpr ObjectRef AUX_ENCODER_STATUS{
            .index    = 0x2022,
            .subIndex = u8{0x03},
            .label    = "auxiliary encoder status",
            .aliases  = {"Auxiliary Encoder Status", "Output Encoder Status"}};

        inline constexpr ObjectRef CALIBRATION_STATUS{.index    = 0x2022,
                                                      .subIndex = u8{0x04},
                                                      .label    = "calibration status",
                                                      .aliases  = {"Calibration Status"}};

        inline constexpr ObjectRef BRIDGE_STATUS{.index    = 0x2022,
                                                 .subIndex = u8{0x05},
                                                 .label    = "bridge status",
                                                 .aliases  = {"Bridge Status"}};

        inline constexpr ObjectRef HARDWARE_STATUS{.index    = 0x2022,
                                                   .subIndex = u8{0x06},
                                                   .label    = "hardware status",
                                                   .aliases  = {"Hardware Status"}};

        inline constexpr ObjectRef COMMUNICATION_STATUS{.index    = 0x2022,
                                                        .subIndex = u8{0x07},
                                                        .label    = "communication status",
                                                        .aliases  = {"Communication Status"}};

        inline constexpr ObjectRef MOTION_STATUS{.index    = 0x2022,
                                                 .subIndex = u8{0x08},
                                                 .label    = "motion status",
                                                 .aliases  = {"Motion Status"}};

        // Command record, 0x2023 in md_1.2, was 0x2003 "System Command" in MDv1.0.0
        inline constexpr ObjectRef CMD_RESET_CONTROLLER{.index    = 0x2023,
                                                        .subIndex = u8{0x08},
                                                        .label    = "reset controller command",
                                                        .aliases  = {"Reset Controller"}};

        inline constexpr ObjectRef CMD_CLEAR_ERRORS{.index    = 0x2023,
                                                    .subIndex = u8{0x0A},
                                                    .label    = "clear errors command",
                                                    .aliases  = {"Clear Errors"}};

        inline constexpr ObjectRef CMD_BLINK_LEDS{.index    = 0x2023,
                                                  .subIndex = u8{0x0B},
                                                  .label    = "blink LEDs command",
                                                  .aliases  = {"Blink LEDs"}};

        inline constexpr ObjectRef CMD_SET_ZERO{.index    = 0x2023,
                                                .subIndex = u8{0x0C},
                                                .label    = "set zero command",
                                                .aliases  = {"Set Zero"}};

        inline constexpr ObjectRef CMD_REINIT_CAN{
            .index    = 0x2023,
            .subIndex = u8{0x0D},
            .label    = "reinitialise CAN command",
            .aliases  = {"Reinitialise CAN", "Run Can Reinit"}};

        // ---- objects carrying the motor configuration -------------------------------------
        // Records the configuration lives in. 0x2000 was "Motor Settings" in MDv1.0.0, the
        // encoder, torque sensor and GPIO records were renumbered wholesale in md_1.2
        inline constexpr std::array<std::string_view, 2> ACTUATOR_CONFIG_NAMES{"Actuator Config",
                                                                               "Motor Settings"};
        inline constexpr std::array<std::string_view, 2> MAIN_ENCODER_NAMES{"Main Encoder"};
        inline constexpr std::array<std::string_view, 2> AUX_ENCODER_NAMES{"Auxiliary Encoder",
                                                                           "Output Encoder"};
        inline constexpr std::array<std::string_view, 2> TORQUE_SENSOR_NAMES{"Torque Sensor"};
        inline constexpr std::array<std::string_view, 2> USER_GPIO_NAMES{"User GPIO"};
        inline constexpr std::array<std::string_view, 2> POSITION_PID_NAMES{
            "Position PID Controller"};
        inline constexpr std::array<std::string_view, 2> VELOCITY_PID_NAMES{
            "Velocity PID Controller"};
        inline constexpr std::array<std::string_view, 2> IMPEDANCE_PD_NAMES{
            "Impedance PD Controller"};

        inline constexpr ObjectRef MOTOR_NAME{.index         = 0x2000,
                                              .subIndex      = u8{0x06},
                                              .label         = "motor name",
                                              .aliases       = {"Motor Name"},
                                              .parentAliases = ACTUATOR_CONFIG_NAMES};

        inline constexpr ObjectRef POLE_PAIRS{.index         = 0x2000,
                                              .subIndex      = u8{0x02},
                                              .label         = "pole pairs",
                                              .aliases       = {"Pole Pairs"},
                                              .parentAliases = ACTUATOR_CONFIG_NAMES};

        inline constexpr ObjectRef MOTOR_SHUTDOWN_TEMP{
            .index         = 0x2000,
            .subIndex      = u8{0x07},
            .label         = "motor shutdown temperature",
            .aliases       = {"Motor Shutdown Temperature"},
            .parentAliases = ACTUATOR_CONFIG_NAMES};

        // the motor calibration mode, not the auxiliary encoder one that shares the short name
        inline constexpr ObjectRef MOTOR_CALIBRATION_MODE{
            .index         = 0x2000,
            .subIndex      = u8{0x08},
            .label         = "motor calibration mode",
            .aliases       = {"Motor Calibration Mode", "Calibration Mode"},
            .parentAliases = ACTUATOR_CONFIG_NAMES};

        inline constexpr ObjectRef MAIN_ENCODER_TYPE{.index         = 0x2001,
                                                     .subIndex      = u8{0x01},
                                                     .label         = "main encoder type",
                                                     .aliases       = {"Type"},
                                                     .parentAliases = MAIN_ENCODER_NAMES};

        inline constexpr ObjectRef MAIN_ENCODER_DIR{.index         = 0x2001,
                                                    .subIndex      = u8{0x02},
                                                    .label         = "main encoder direction",
                                                    .aliases       = {"Direction"},
                                                    .parentAliases = MAIN_ENCODER_NAMES};

        inline constexpr ObjectRef AUX_ENCODER_TYPE{.index         = 0x2002,
                                                    .subIndex      = u8{0x01},
                                                    .label         = "auxiliary encoder type",
                                                    .aliases       = {"Type"},
                                                    .parentAliases = AUX_ENCODER_NAMES};

        inline constexpr ObjectRef AUX_ENCODER_MODE{.index         = 0x2002,
                                                    .subIndex      = u8{0x03},
                                                    .label         = "auxiliary encoder mode",
                                                    .aliases       = {"Mode"},
                                                    .parentAliases = AUX_ENCODER_NAMES};

        // moved from subindex 2 in MDv1.0.0 to subindex 4 in md_1.2
        inline constexpr ObjectRef AUX_ENCODER_CALIBRATION_MODE{
            .index         = 0x2002,
            .subIndex      = u8{0x04},
            .label         = "auxiliary encoder calibration mode",
            .aliases       = {"Calibration Mode"},
            .parentAliases = AUX_ENCODER_NAMES};

        inline constexpr ObjectRef TORQUE_SENSOR_TYPE{.index         = 0x2003,
                                                      .subIndex      = u8{0x01},
                                                      .label         = "torque sensor type",
                                                      .aliases       = {"Type"},
                                                      .parentAliases = TORQUE_SENSOR_NAMES};

        inline constexpr ObjectRef GPIO_MODE{
            .index         = 0x2024,
            .subIndex      = u8{0x01},
            .label         = "user GPIO mode",
            .aliases       = {"GPIO Mode", "User GPIO Configuration"},
            .parentAliases = USER_GPIO_NAMES};

        inline constexpr ObjectRef POSITION_PID_KP{.index         = 0x2010,
                                                   .subIndex      = u8{0x01},
                                                   .label         = "position PID kp",
                                                   .aliases       = {"Kp"},
                                                   .parentAliases = POSITION_PID_NAMES};
        inline constexpr ObjectRef POSITION_PID_KI{.index         = 0x2010,
                                                   .subIndex      = u8{0x02},
                                                   .label         = "position PID ki",
                                                   .aliases       = {"Ki"},
                                                   .parentAliases = POSITION_PID_NAMES};
        inline constexpr ObjectRef POSITION_PID_KD{.index         = 0x2010,
                                                   .subIndex      = u8{0x03},
                                                   .label         = "position PID kd",
                                                   .aliases       = {"Kd"},
                                                   .parentAliases = POSITION_PID_NAMES};
        inline constexpr ObjectRef POSITION_PID_WINDUP{.index         = 0x2010,
                                                       .subIndex      = u8{0x04},
                                                       .label         = "position PID windup",
                                                       .aliases       = {"Integral Limit", "Windup"},
                                                       .parentAliases = POSITION_PID_NAMES};

        inline constexpr ObjectRef VELOCITY_PID_KP{.index         = 0x2011,
                                                   .subIndex      = u8{0x01},
                                                   .label         = "velocity PID kp",
                                                   .aliases       = {"Kp"},
                                                   .parentAliases = VELOCITY_PID_NAMES};
        inline constexpr ObjectRef VELOCITY_PID_KI{.index         = 0x2011,
                                                   .subIndex      = u8{0x02},
                                                   .label         = "velocity PID ki",
                                                   .aliases       = {"Ki"},
                                                   .parentAliases = VELOCITY_PID_NAMES};
        inline constexpr ObjectRef VELOCITY_PID_KD{.index         = 0x2011,
                                                   .subIndex      = u8{0x03},
                                                   .label         = "velocity PID kd",
                                                   .aliases       = {"Kd"},
                                                   .parentAliases = VELOCITY_PID_NAMES};
        inline constexpr ObjectRef VELOCITY_PID_WINDUP{.index         = 0x2011,
                                                       .subIndex      = u8{0x04},
                                                       .label         = "velocity PID windup",
                                                       .aliases       = {"Integral Limit", "Windup"},
                                                       .parentAliases = VELOCITY_PID_NAMES};

        inline constexpr ObjectRef IMPEDANCE_PD_KP{.index         = 0x2012,
                                                   .subIndex      = u8{0x01},
                                                   .label         = "impedance PD kp",
                                                   .aliases       = {"Kp"},
                                                   .parentAliases = IMPEDANCE_PD_NAMES};
        inline constexpr ObjectRef IMPEDANCE_PD_KD{.index         = 0x2012,
                                                   .subIndex      = u8{0x02},
                                                   .label         = "impedance PD kd",
                                                   .aliases       = {"Kd"},
                                                   .parentAliases = IMPEDANCE_PD_NAMES};

        // Firmware version, used to pick the .eds revision before anything else is read.
        // md_1.2 keeps it at 0x2021:02 "Firmware Version", MDv1.0.0 had it at 0x200A:03 "Version"
        inline constexpr std::array<std::string_view, 2> FIRMWARE_INFO_NAMES{"Firmware Info"};

        inline constexpr ObjectRef FIRMWARE_VERSION{
            .index         = 0x2021,
            .subIndex      = u8{0x02},
            .label         = "firmware version",
            .aliases       = {"Firmware Version", "Version"},
            .parentAliases = FIRMWARE_INFO_NAMES};

        inline constexpr ObjectRef TORQUE_BANDWIDTH_CFG{.index         = 0x2000,
                                                        .subIndex      = u8{0x05},
                                                        .label         = "torque bandwidth",
                                                        .aliases       = {"Torque Bandwidth"},
                                                        .parentAliases = ACTUATOR_CONFIG_NAMES};

        /// @brief Format an object address for diagnostics
        inline std::string addressToString(u16 index, const std::optional<u8>& subIndex)
        {
            std::stringstream ss;
            ss << "0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
               << index;
            if (subIndex.has_value())
                ss << ":" << std::setw(2) << (unsigned)subIndex.value();
            return ss.str();
        }

        /// @brief Format the address of a known object for diagnostics
        inline std::string addressToString(const ObjectRef& ref)
        {
            return addressToString(ref.index, ref.subIndex);
        }

        /// @brief Report an object drifting from its expected address only once per run, the
        /// objects are resolved on every call and the message carries no new information after
        /// the first time
        /// @param ref object that drifted
        /// @return true when this object has not been reported yet
        inline bool firstReportOf(const ObjectRef& ref)
        {
            static std::mutex                 mutex;
            static std::set<const ObjectRef*> reported;

            const std::lock_guard<std::mutex> lock(mutex);
            return reported.insert(&ref).second;
        }

        /// @brief Locate an object in the dictionary parsed from the .eds file
        ///
        /// The index is tried first and the name of whatever sits there confirms the identity.
        /// When the two disagree the known names are searched across the whole dictionary, which
        /// covers an object that moved to another index. An object found at its index under an
        /// unknown name is accepted as a rename, as long as none of its known names turns up
        /// anywhere else.
        ///
        /// @param od object dictionary parsed from the .eds file
        /// @param ref object to locate
        /// @param log logger used to report a drifting or missing object
        /// @return entry of the object, nullptr when the .eds does not define it at all
        inline EDSEntry* resolveObject(EDSObjectDictionary& od,
                                       const ObjectRef&     ref,
                                       const Logger&        log)
        {
            EDSEntry* atAddress = nullptr;
            if (od.hasEntry(ref.index))
            {
                EDSEntry& entry = od[ref.index];
                if (!ref.subIndex.has_value())
                    atAddress = &entry;
                else if (entry.hasSubEntry(ref.subIndex.value()))
                    atAddress = &entry[ref.subIndex.value()];
            }

            const auto carriesKnownName = [](const EDSEntry& entry, const auto& names)
            {
                for (const auto& name : names)
                {
                    if (!name.empty() && namesMatch(name, entry.getEntryMetaData().parameterName))
                        return true;
                }
                return false;
            };

            // expected address holding the expected object
            if (atAddress != nullptr && carriesKnownName(*atAddress, ref.aliases))
                return atAddress;

            // the sub-entry is identified inside its own record, its name may repeat elsewhere
            if (ref.subIndex.has_value() && !ref.parentAliases.front().empty())
            {
                EDSEntry* record = od.hasEntry(ref.index) ? &od[ref.index] : nullptr;
                if (record == nullptr || !carriesKnownName(*record, ref.parentAliases))
                    record = findByAnyName(od, ref.parentAliases);

                if (record != nullptr)
                {
                    // the subindex is deliberately not used as a fallback here, records get
                    // reshuffled between revisions (the auxiliary encoder calibration mode moved
                    // from subindex 2 to 4) and landing on the wrong member would silently write
                    // one parameter into another
                    EDSEntry* member = findSubByAnyName(*record, ref.aliases);

                    if (member != nullptr)
                    {
                        const auto& memberAddress = member->getEntryMetaData().address;
                        if (memberAddress.first != ref.index ||
                            memberAddress.second != ref.subIndex)
                        {
                            if (firstReportOf(ref))
                                log.warn("%s is at %s in this .eds, expected %s - the .eds "
                                         "revision differs from the one candletool was built "
                                         "against",
                                         ref.label.data(),
                                         addressToString(memberAddress.first,
                                                         memberAddress.second)
                                             .c_str(),
                                         addressToString(ref).c_str());
                        }
                        return member;
                    }
                }

                log.error("Could not locate the %s object (%s) in the .eds file!",
                          ref.label.data(),
                          addressToString(ref).c_str());
                return nullptr;
            }

            // object moved to another index in this .eds revision
            for (const auto& alias : ref.aliases)
            {
                if (alias.empty())
                    continue;

                auto found = od.getEntryByName(alias);
                if (!found.has_value())
                    continue;

                EDSEntry&   entry       = found.value().get();
                const auto& foundAddress = entry.getEntryMetaData().address;

                if (firstReportOf(ref))
                    log.warn(
                         "%s is at %s in this .eds, expected %s - the .eds revision differs from "
                         "the one candletool was built against",
                         ref.label.data(),
                         addressToString(foundAddress.first, foundAddress.second).c_str(),
                         addressToString(ref).c_str());
                return &entry;
            }

            // object kept its address but was renamed, none of the known names is used elsewhere
            if (atAddress != nullptr)
            {
                if (firstReportOf(ref))
                    log.warn("%s (%s) is named '%s' in this .eds, accepting it by address",
                             ref.label.data(),
                             addressToString(ref).c_str(),
                             atAddress->getEntryMetaData().parameterName.c_str());
                return atAddress;
            }

            log.error("Could not locate the %s object (%s) in the .eds file!",
                      ref.label.data(),
                      addressToString(ref).c_str());
            return nullptr;
        }
    }  // namespace md_objects
}  // namespace mab

#endif  // MD_OBJECTS_HPP
