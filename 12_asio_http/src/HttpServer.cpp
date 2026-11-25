#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "server.hpp"
#include <filesystem>

int main(int argc, char* argv[])
{
	try
	{
		std::filesystem::path path = std::filesystem::current_path() / "res";

		// 使用 std::cout 输出拼接后的路径
		std::cout << "Path: " << path.string() << '\n';
		std::cout << "Usage: http_server <127.0.0.1> <8080> "<< path.string() <<"\n";
		// Initialise the server.
		http::server::server s("127.0.0.1", "8080", path.string());

		// Run the server until stopped.
		s.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}

