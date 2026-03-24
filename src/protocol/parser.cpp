#include <pkmpch.h>
#include "parser.h"

namespace pkm::protocol {
    
    static std::vector<std::string_view> split(std::string_view s, char delim) {
        std::vector<std::string_view> tokens;
        size_t start = 0;
        size_t end = s.find(delim);
        
        while (end != std::string_view::npos) {
            std::string_view token = s.substr(start, end - start);
            
            // remove the carriage return if it exists
            if (!token.empty() && token.back() == '\r') {
                token.remove_suffix(1); 
            }
            
            tokens.push_back(token);
            start = end + 1;
            end = s.find(delim, start);
        }
        
        // grab the final token after the last delimiter
        std::string_view last_token = s.substr(start);
        if (!last_token.empty() && last_token.back() == '\r') {
            last_token.remove_suffix(1);
        }
        tokens.push_back(last_token);
        
        return tokens;
    }

    std::vector<Message> parse_message(std::string_view raw) {
        if (raw.empty()) return {};
        
        std::vector<std::string_view> lines = split(raw, '\n');
        
        std::string current_room;
        std::vector<Message> msgs;

        for (std::string_view line : lines) {
            if (line.empty()) continue;
            
            // room context line
            if (line[0] == '>') {
                current_room = std::string(line.substr(1)); 
                continue;
            }
            
            if (line[0] == '|') {
                std::vector<std::string_view> tokens = split(line.substr(1), '|');
                if (tokens.empty()) continue;

                Message msg;
                msg.room_id = current_room;
                msg.type    = std::string(tokens[0]);
                
                // convert views back into owned strings for the struct
                for (size_t i = 1; i < tokens.size(); ++i) {
                    msg.args.emplace_back(tokens[i]);
                }

                msgs.push_back(std::move(msg));
            }
        }

        return msgs;
    }

    Ident parse_ident(std::string_view ident) {
        if (ident.empty()) {
            PK_ERROR("Empty ident passed!");
            return Ident();
        }

        size_t pos = ident.find(':');

        if (pos != std::string_view::npos) {
            std::string_view meta_data = ident.substr(0, pos);

            auto split_pos = std::find_if(ident.begin() + pos + 1, ident.end(),
                             [](char c) { return c != ' '; });

            if (split_pos == ident.end()) {
               PK_ERROR("No pokemon name in ident {}!", ident);
               return Ident();
            }

            size_t start_idx = std::distance(ident.begin(), split_pos); 
            std::string_view pkm_name = ident.substr(start_idx);
            
            // verify size before pulling the side
            if (meta_data.size() == 2) { 
                return {'\0', std::string(meta_data), std::string(pkm_name)};
            } else if (meta_data.size() == 3) { 
                return {meta_data[2], std::string(meta_data.substr(0, 2)), std::string(pkm_name)};
            } else {
                PK_ERROR("Unexpected ident size: {}", ident);
                return Ident();
            }
            
        } else {
            PK_ERROR("Failed to parse ident {}!", ident);
            return Ident();
        }
    }
}