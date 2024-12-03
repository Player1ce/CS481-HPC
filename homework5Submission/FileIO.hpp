#ifndef CS481_HPC_FILEIO_HPP
#define CS481_HPC_FILEIO_HPP

// STL
#include <boost/filesystem.hpp>
// #include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <sstream>

namespace util::file_io
{
    /** Reads from a file at the given filepath and returns the lines of the file as an std::vector object.
     *  Returns an empty vector if the file could not be accessed or there was an error.
     *
     * @param filePath the path to the file, either absolute or relative
     * @return the lines of the file, seperated by newline
     */
    [[maybe_unused]] static std::vector<std::string> readFile(const std::string& filePath) noexcept
    {
        try
        {
            std::ifstream file;
            file.open(filePath, std::ios::in);

            std::vector<std::string> result;
            std::string line;
            while(std::getline(file, line))
                result.push_back(line);

            file.close();

            return result;
        }
        catch(std::exception& e)
        {
            return {};
        }
    }

    /** Similar to the above function, this function reads an entire file into a string.
     *
     * @param filePath the path to the file, either absolute or relative
     * @return the contents of the file
     */
    [[maybe_unused]] static std::string readFullFile(const std::string& filePath) noexcept
    {
        try
        {
            std::ifstream stream(filePath);
            std::stringstream buffer;
            buffer << stream.rdbuf();

            return buffer.str();
        }
        catch(std::exception& e)
        {
            return "";
        }

    }

    /** Writes the provided lines to a file. Returns true if the operation succeeded.
     *
     * @param filePath the path to the file, either absolute or relative
     * @param lines the lines to write
     * @return true if there were no exceptions
     */
    [[maybe_unused]] static bool writeTofile(const std::string& filePath, const std::vector<std::string>& lines) noexcept
    {
        try
        {
            std::ofstream file;
            file.open(filePath, std::ios::out);

            for(const auto& line : lines)
                file << line << '\n';

            file.close();

            return true;
        }
        catch(std::exception& e)
        {
            return false;
        }
    }

    /** Appends the provided lines to a file. No newlines are added before appending. Returns true if the operation succeeded.
     *
     * @param filePath the path to the file, either absolute or relative
     * @param lines the lines to write
     * @return true if there were no exceptions
     */
    [[maybe_unused]] static bool appendToFile(const std::string& filePath, const std::vector<std::string>& lines) noexcept
    {
        try
        {
            std::ofstream file;
            file.open(filePath, std::ios::app);

            for(const auto& line : lines)
                file << line << '\n';

            file.close();

            return true;
        }
        catch(std::exception& e)
        {
            return false;
        }
    }

    /** Appends the provided line to a file. No newlines are added before appending. Returns true if the operation succeeded.
     *
     * @param filePath the path to the file, either absolute or relative
     * @param line the line to write
     * @return true if there were no exceptions
     */
    static bool appendToFile(const std::string& filePath, const std::string& line) noexcept {
        return appendToFile(filePath, std::vector<std::string>({line}));
    }

    /** Checks if a file exists at the given path.
     *
     * @param filePath the path to the file, either absolute or relative
     * @return true if the file exists
     */
    [[maybe_unused]] static bool fileExists(const std::string& filePath) noexcept
    {
        try
        {
            struct stat buffer;
            return (stat(filePath.c_str(), &buffer) == 0);
        }
        catch(std::exception& e)
        {
            return false;
        }
    }

    /** Uses directory iteration to list the files in a directory.
     *
     * @param path the path to the directory
     * @return a vector of string filenames within the directory
     */
    [[maybe_unused]] static std::vector<std::string> listDirectory(const std::string& path) noexcept
    {
        try
        {
            std::vector<std::string> fileNames;

            for(const auto& file : boost::filesystem::directory_iterator(boost::filesystem::path(path)))
            // for(const auto& file : std::filesystem::directory_iterator(path))
            {
                fileNames.push_back(file.path().string());
            }

            return fileNames;
        }
        catch(std::exception& e)
        {
            return std::vector<std::string>();
        }
    }

    /** Removes a file at the specified path
     *
     * @param fileName the path to the file to delete
     * @return true on success, false on error
     */
    [[maybe_unused]] static bool removeFile(const std::string& fileName) noexcept
    {
        try
        {
            std::remove(fileName.c_str());

            return true;
        }
        catch(std::exception& e)
        {
            return false;
        }
    }
}

#endif //CS481_HPC_FILEIO_HPP
