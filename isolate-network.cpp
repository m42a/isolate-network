#include <string>
#include <type_traits>

#include <fcntl.h>
#include <sys/stat.h>
#include <sched.h>
#include <stdlib.h>
#include <grp.h>
#include <net/if.h>
#include <unistd.h>

#include <netlink/netlink.h>
#include <netlink/route/link.h>

using namespace std::literals;

namespace
{
	using id_t = std::common_type_t<uid_t, gid_t>;

	[[noreturn]] void die(const char *name)
	{
		perror(name);
		exit(EXIT_FAILURE);
	}

	[[noreturn]] void die(std::string name)
	{
		die(name.c_str());
	}

	std::string get_id_mapping(id_t id)
	{
		auto id_str = std::to_string(id);
		return id_str + " " + id_str + " 1\n";
	}

	void write_file(const char *file, std::string_view contents, bool allow_missing)
	{
		auto die_op = [&](std::string op) {
			die(op + "(\"" + file + "\")");
		};
		auto fd = open(file, O_WRONLY);
		if (fd == -1)
		{
			if (allow_missing && errno == ENOENT)
				return;
			die_op("open"s);
		}
		auto bytes_written = write(fd, contents.data(), contents.size());
		if (bytes_written != contents.size())
			die_op("write"s);
		close(fd);
	}

	void write_id_mapping(const char *file, id_t id)
	{
		write_file(file, get_id_mapping(id), false);
	}
}

int main(int argc, char **argv)
{
	// These need to be done before we create the new user namespace
	auto uid = getuid();
	auto gid = getgid();

	auto ret = unshare(CLONE_NEWUSER | CLONE_NEWNET);
	if (ret == -1)
		die("unshare");

	// Set up the new user namespace
	write_id_mapping("/proc/self/uid_map", uid);
	write_file("/proc/self/setgroups", "deny\n"sv, true);
	write_id_mapping("/proc/self/gid_map", gid);

	// Bring up the loopback device (equivalent to "ip link set lo up")
	auto sock = nl_socket_alloc();
	if (!sock)
		die("nl_socket_alloc");
	ret = nl_connect(sock, NETLINK_ROUTE);
	if (ret)
		die("nl_connect");

	rtnl_link *lo_link;
	ret = rtnl_link_get_kernel(sock, 0, "lo", &lo_link);
	if (ret)
		die("rtnl_link_get_kernel");

	auto link_up = rtnl_link_alloc();
	if (!link_up)
		die("rtnl_link_alloc");
	rtnl_link_set_flags(link_up, IFF_UP);


	ret = rtnl_link_change(sock, lo_link, link_up, 0);
	if (ret)
		die("rtnl_link_change");

	// If we have a command, run it
	if (argc > 1)
	{
		execvp(argv[1], &argv[1]);
		die("execvp");
	}

	const char *shell = getenv("SHELL");
	if (!shell)
		shell = "/bin/sh";

	execl(shell, shell, nullptr);
	die("execl");
}
