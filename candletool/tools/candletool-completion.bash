_candletool_completions()
{
    local current="${COMP_WORDS[COMP_CWORD]}"
    local previous="${COMP_WORDS[COMP_CWORD-1]}"
    local suggestions=""
    local global_flags="-h --help -d --datarate -i --id --bus --device -v --verbosity --version -s --silent --log"
    local flags_with_args="-i --id --bus --datarate -d --device -v --verbosity -p --path -e --encoder -f --mabfile -r --recovery --new_id --new_datarate --new_timeout --index --subindex --value"

    if [[ "$previous" == "-p" || "$previous" == "--path" || "$previous" == "-f" || "$previous" == "--mabfile" || "$previous" == "upload" || "$previous" == "download" ]]; then
        compopt -o filenames 2>/dev/null
        COMPREPLY=( $(compgen -f -- "$current") )
        return 0
    fi

    
    if [[ "$current" == -* ]]; then
        suggestions="$global_flags"
    else
        suggestions=""
    fi

    local positionCounter=0
    local nonFlagWords=()

    for ((i=1; i<COMP_CWORD; i++)); do
        if [[ ! "${COMP_WORDS[i]}" == -* ]]; then
            nonFlagWords+=("${COMP_WORDS[i]}")
            ((positionCounter++))
        else
            if [[ "$flags_with_args" == *"${COMP_WORDS[i]}"* ]]; then
                ((i++))
            fi
        fi
    done

    local deviceType="${nonFlagWords[0]}"
    local functionName="${nonFlagWords[1]}"
    local subCommand="${nonFlagWords[2]}"
    local deepCommand="${nonFlagWords[3]}"

    if [[ $positionCounter -eq 0 && ${COMP_CWORD} -eq 1 ]]; then
        local devices="candle md pds mdco update"
        suggestions="${suggestions} ${devices}"
        COMPREPLY=($(compgen -W "$suggestions" -- "$current"))
        return 0
    fi

    if [[ $positionCounter -eq 1 ]]; then
        local functions=""

        case "$deviceType" in
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
            update)
                [[ "$current" == -* ]] && functions="-y --yes"
                ;;
        esac

        suggestions="${suggestions} ${functions}"
        COMPREPLY=($(compgen -W "$suggestions" -- "$current"))
        return 0
    fi

    if [[ $positionCounter -eq 2 ]]; then
        local exclusive_flags=""
        local subcommands=""

        case "$deviceType" in
            candle)
                case "$functionName" in
                    update)
                        exclusive_flags="-p --path"
                        ;;
                esac
                ;;
            md)
                case "$functionName" in
                    can)
                        exclusive_flags="--new_id --new_datarate --new_timeout --save"
                        ;;
                    calibration)
                        exclusive_flags="-e --encoder"
                        ;;
                    config)
                        subcommands="download upload factory-reset verify"
                        ;;
                    register)
                        subcommands="read write"
                        ;;
                    test)
                        subcommands="absolute relative velocity encoder"
                        ;;
                    update)
                        exclusive_flags="-p --path -r --recovery --force-erase"
                        ;;
                esac
                ;;
            mdco)
                case "$functionName" in
                    can)
                        exclusive_flags="--new_id"
                        ;;
                    config)
                        subcommands="download upload"
                        ;;
                    encoder)
                        subcommands="display"
                        ;;
                    sdo)
                        subcommands="read write"
                        ;;
                    calibration)
                        exclusive_flags="-e"
                        ;;
                    test)
                        subcommands="move"
                        ;;
                esac
                ;;
            pds)
                case "$functionName" in
                    update)
                        exclusive_flags="-f --mabfile -r --recovery"
                        ;;
                    can)
                        subcommands="id data"
                        ;;
                    ps|br|ic)
                        ;;
                esac
                ;;
        esac

        suggestions="${suggestions} ${exclusive_flags} ${subcommands}"
        COMPREPLY=($(compgen -W "$suggestions" -- "$current"))
        return 0
    fi

    if [[ $positionCounter -eq 3 ]]; then
        local exclusive_flags=""
        local subcommands=""

        case "$deviceType" in
            mdco)
                case "$functionName" in
                    test)
                        [[ "$subCommand" == "move" ]] && subcommands="absolute relative velocity"
                        ;;
                    sdo)
                        [[ "$subCommand" == "read" ]] && exclusive_flags="--index --subindex"
                        [[ "$subCommand" == "write" ]] && exclusive_flags="--index --subindex --value"
                        ;;
                esac
                ;;
            pds)
                case "$functionName" in
                    ps)
                        subcommands="info enable disable set_ovc_level get_ovc_level set_ovc_delay get_ovc_delay get_delivered_energy reset_delivered_energy set_temp_limit get_temp_limit set_br get_br set_br_trigger get_br_trigger set_auto_start get_auto_start clear"
                        ;;
                    br)
                        subcommands="info set_temp_limit get_temp_limit clear"
                        ;;
                    ic)
                        subcommands="info enable disable set_ovc_level get_ovc_level set_ovc_delay get_ovc_delay set_temp_limit get_temp_limit clear"
                        ;;
                esac
                ;;
        esac

        suggestions="${suggestions} ${exclusive_flags} ${subcommands}"
        COMPREPLY=($(compgen -W "$suggestions" -- "$current"))
        return 0
    fi

    if [[ $positionCounter -ge 4 ]]; then
        case "$deviceType" in
            mdco)
                case "$functionName" in
                    test)
                        [[ "$subCommand" == "move" && ("$deepCommand" == "absolute" || "$deepCommand" == "relative" || "$deepCommand" == "velocity") ]] && \
                        suggestions="${suggestions}"
                        ;;
                esac
                ;;
        esac
        
        COMPREPLY=($(compgen -W "$suggestions" -- "$current"))
        return 0
    fi
}

complete -F _candletool_completions candletool