_candletool_completions()
{
    local current="${COMP_WORDS[COMP_CWORD]}"

    if [[ ${COMP_CWORD} == 1 ]]; then
        local devices="candle md pds mdco"
        COMPREPLY=($(compgen -W "$devices" -- "$current"))
        return 0
    fi

    local device_type="${COMP_WORDS[1]}"

    if [[ ${COMP_CWORD} == 2 ]]; then
        local functions=""

        case "$device_type" in
            candle)
                functions="update version"
                ;;
            md)
                functions="blink can calibration clear config discover save info register reset test update version zero"
                ;;
            pds)
                functions="discover info update can setup_cfg setup_interactive read_cfg save set_battery_level set_shutdown_time set_br get_br set_br_trigger get_br_trigger disable ps br ic"
                ;;
            mdco)
                functions="blink can config clear discover encoder sdo reset calibration info save test"
                ;;
            *)
                functions=""
                ;;
        esac

        COMPREPLY=($(compgen -W "$functions" -- "$current"))
        return 0
    fi

    if [[ "$device_type" == "pds" ]]; then
        if [[ ${COMP_CWORD} == 4 ]]; then
            local function_name="${COMP_WORDS[2]}"
            local pds_function=""

            case "$function_name" in
                ps)
                    pds_function="info enable disable set_ovc_level get_ovc_level set_ovc_delay get_ovc_delay get_delivered_energy reset_delivered_energy set_temp_limit get_temp_limit set_br get_br set_br_trigger get_br_trigger set_auto_start get_auto_start clear"
                    ;;
                ic)
                    pds_function="info enable disable set_ovc_level get_ovc_level set_ovc_delay get_ovc_delay set_temp_limit get_temp_limit clear"
                    ;;
                br)
                    pds_function="info set_temp_limit get_temp_limit clear"
                    ;;
                *)
                    pds_function=""
                    ;;
            esac

            COMPREPLY=($(compgen -W "$pds_function" -- "$current"))
            return 0
        fi
    fi
}

complete -F _candletool_completions candletool