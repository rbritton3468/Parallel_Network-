std::string runCommandAndGetOutput(std::string_view command) {
    std::string output;
    FILE* pipe = popen(std::string(command).c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to execute the following command : " << command << std::endl;
        return "";
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    auto returnCode = pclose(pipe);
    // if (returnCode != 0) {
    //     std::cerr << "Command retunred an error: " << returnCode << std::endl;
    // }

    return output;
}