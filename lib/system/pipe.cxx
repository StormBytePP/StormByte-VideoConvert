#include "pipe.hxx"

#include <unistd.h>

Alchemist::System::Pipe::Pipe() {
	init();
}

Alchemist::System::Pipe::~Pipe() {
	close_read();
	close_write();
}

void Alchemist::System::Pipe::bind_read(int dest) {
	bind(m_fd[0], dest);
}

void Alchemist::System::Pipe::bind_write(int dest) {
	bind(m_fd[1], dest);
}

void Alchemist::System::Pipe::close_read() {
	close(m_fd[0]);
}

void Alchemist::System::Pipe::close_write() {
	close(m_fd[1]);
}

int Alchemist::System::Pipe::poll(int timeout) const {
	return ::poll(m_fd_data, 2, timeout);
}

bool Alchemist::System::Pipe::has_read_event(unsigned short event) const {
	return (m_fd_data[0].revents & event) == event;
}

bool Alchemist::System::Pipe::has_write_event(unsigned short event) const {
	return (m_fd_data[1].revents & event) == event;
}

Alchemist::System::Pipe& Alchemist::System::Pipe::operator<<(const std::string& data) {
	write(data);
	return *this;
}

std::optional<std::string>& Alchemist::System::Pipe::operator>>(std::optional<std::string>& out) const {
	std::optional<std::string> data = read();
	if (data) {
		if (out)
			out = *out + *data;
		else
			out = *data;
	}
	return out;
}

void Alchemist::System::Pipe::write(const std::string& str) {
	bool retry = true;
	do {
		poll(100);
		if (has_write_event(POLLHUP)) {
			retry = false;
		}
		else if (has_write_event(POLLOUT)) {
			::write(m_fd[1], str.c_str(), sizeof(char) * str.length());
			retry = false;
		}
	} while (retry);
}

std::optional<std::string> Alchemist::System::Pipe::read() const {
	std::optional<std::string> result;
	bool retry = true;
	do {
		poll(100);
		ssize_t bytes;
		if (has_read_event(POLLIN)) {
			char buffer[MAX_BYTES];
			std::string data = "";
			while ((bytes = ::read(m_fd[0], buffer, MAX_BYTES)) > 0) {
				data += std::string(buffer, bytes);
			};
			if (!data.empty()) {
				result = std::move(data);
			}
			retry = false;
		}
		else if (has_read_event(POLLHUP)) {
			retry = false;
		}
	} while(retry);
	return result;
}

void Alchemist::System::Pipe::bind(int& src, int dest) {
	dup2(src, dest);
	close(src); // Here maybe use ::close instead
}

void Alchemist::System::Pipe::close(int& fd) {
	::close(fd);
}

void Alchemist::System::Pipe::init() {
	pipe(m_fd);
	m_fd_data[0].fd = m_fd[0];
	m_fd_data[0].events = POLLIN;
	m_fd_data[0].revents = 0;
	m_fd_data[1].fd = m_fd[1];
	m_fd_data[1].events = POLLOUT;
	m_fd_data[1].revents = 0;
}