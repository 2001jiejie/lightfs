#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>

const std::string FILE_SYSTEM_NAME = "light.fs";
const size_t FILE_SYSTEM_SIZE = 256 * 1024 * 1024; // 256MB
const size_t META_SIZE = 56 * 1024 * 1024;         // 56MB
const size_t DATA_SIZE = 200 * 1024 * 1024;        // 200MB
const size_t BLOCK_SIZE = 1 * 1024 * 1024;         // 1MB

struct FileEntry
{
    char name[256];
    size_t size;
    std::string content;
    size_t blockIndex;
};

class LightFS
{
private:
    std::vector<FileEntry> fileEntries;
    size_t usedBlocks;

public:
    LightFS() : usedBlocks(0)
    {
        // 初始化文件系统
        std::ofstream fs(FILE_SYSTEM_NAME, std::ios::binary | std::ios::trunc);
        fs.seekp(FILE_SYSTEM_SIZE - 1);
        fs.write("", 1);
        fs.close();
    }

    void createFile(const std::string& filename)
    {
        if (usedBlocks >= DATA_SIZE / BLOCK_SIZE)
        {
            std::cout << "没有足够的空间来创建文件。" << std::endl;
            return;
        }
        FileEntry entry;
        strncpy(entry.name, filename.c_str(), sizeof(entry.name));
        entry.size = 0;
        entry.blockIndex = usedBlocks;
        usedBlocks++;
        fileEntries.push_back(entry);
        std::cout << "文件 " << filename << " 创建成功。" << std::endl;
    }

    void deleteFile(const std::string& filename)
    {
        for (auto it = fileEntries.begin(); it != fileEntries.end(); ++it)
        {
            if (std::string(it->name) == filename)
            {
                usedBlocks--; // 释放文件占用的块
                fileEntries.erase(it);
                std::cout << "文件 " << filename << " 删除成功。" << std::endl;
                return;
            }
        }
        std::cout << "未找到文件 " << filename << "。" << std::endl;
    }

    void listFiles()
    {
        std::cout << "文件列表：" << std::endl;
        for (const auto& entry : fileEntries)
        {
            std::cout << entry.name << " (大小: " << entry.size << " 字节)" << std::endl;
        }
    }

    void writeFile(const std::string& filename, const std::string& content)
    {
        for (auto& entry : fileEntries)
        {
            if (entry.name == filename)
            {
                entry.content = content;
                entry.size = content.size();
                std::cout << "写入内容到文件 " << filename << " 成功。" << std::endl;
                return;
            }
        }
        std::cout << "未找到文件 " << filename << "。" << std::endl;
    }

    void showStats()
    {
        size_t freeBlocks = (DATA_SIZE / BLOCK_SIZE) - usedBlocks;
        std::cout << "已用空间: " << usedBlocks * BLOCK_SIZE / (1024 * 1024) << " MB" << std::endl;
        std::cout << "空闲空间: " << freeBlocks * BLOCK_SIZE / (1024 * 1024) << " MB" << std::endl;
    }

    void readFile(const std::string& filename)
    {
        for (const auto& entry : fileEntries)
        {
            if (entry.name == filename)
            {
                std::cout << "读取文件 " << filename << " 的内容: [" << entry.content << "]" << std::endl;
                return;
            }
        }
        std::cout << "未找到文件 " << filename << "。" << std::endl;
    }

    void renameFile(const std::string& oldName, const std::string& newName)
    {
        for (auto& entry : fileEntries)
        {
            if (std::string(entry.name) == oldName)
            {
                strncpy(entry.name, newName.c_str(), sizeof(entry.name));
                std::cout << "文件 " << oldName << " 重命名为 " << newName << " 成功。" << std::endl;
                return;
            }
        }
        std::cout << "未找到文件 " << oldName << "。" << std::endl;
    }

    void processCommand(const std::string& command)
    {
        std::istringstream iss(command);
        std::string action;
        iss >> action;

        if (action == "create")
        {
            std::string filename;
            iss >> filename;
            createFile(filename);
        }
        else if (action == "delete")
        {
            std::string filename;
            iss >> filename;
            deleteFile(filename);
        }
        else if (action == "list")
        {
            listFiles();
        }
        else if (action == "write")
        {
            std::string filename, content;
            iss >> filename;
            std::getline(iss, content);
            writeFile(filename, content.substr(1)); // 去掉前面的空格
        }
        else if (action == "read")
        {
            std::string filename;
            iss >> filename;
            readFile(filename);
        }
        else if (action == "stats")
        {
            showStats();
        }
        else if (action == "rename")
        {
            std::string oldName, newName;
            iss >> oldName >> newName;
            renameFile(oldName, newName);
        }
        else if (action == "export")
        {
            std::string filename, exportPath;
            iss >> filename >> exportPath;
            exportFile(filename, exportPath);
        }
        else if (action == "import")
        {
            std::string importPath;
            iss >> importPath;
            importFile(importPath);
        }
        else
        {
            std::cout << "未知命令: " << action << std::endl;
        }
    }

    void importFile(const std::string& importPath)
    {
        std::ifstream inFile(importPath);
        if (inFile.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
            std::string filename = "imported_file"; // 可以根据需要修改文件名
            createFile(filename);                   // 创建文件
            writeFile(filename, content);           // 写入内容
            inFile.close();
            std::cout << "文件 " << importPath << " 导入成功，文件名为 " << filename << "。" << std::endl;
        }
        else
        {
            std::cout << "无法打开文件 " << importPath << " 进行读取。" << std::endl;
        }
    }

    void exportFile(const std::string& filename, const std::string& exportPath)
    {
        for (const auto& entry : fileEntries)
        {
            if (entry.name == filename)
            {
                std::ofstream outFile(exportPath);
                if (outFile.is_open())
                {
                    outFile << entry.content; // 写入内容到外部文件
                    outFile.close();
                    std::cout << "文件 " << filename << " 导出成功到 " << exportPath << "。" << std::endl;
                }
                else
                {
                    std::cout << "无法打开文件 " << exportPath << " 进行写入。" << std::endl;
                }
                return;
            }
        }
        std::cout << "未找到文件 " << filename << "。" << std::endl;
    }
};

int main()
{
    LightFS fs;
    std::string command;

    std::cout << "欢迎使用 LightFS 文件系统！" << std::endl;
    std::cout << "可用命令: create <filename>, delete <filename>, list, write <filename> <content>, read <filename>, stats, rename <oldname> <newname>" << std::endl;

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, command);
        if (command == "exit")
        {
            break;
        }
        fs.processCommand(command);
    }

    return 0;
}
