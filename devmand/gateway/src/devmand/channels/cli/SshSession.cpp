// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/SshSession.h>
#include <libssh/libssh.h>
#include <experimental/optional>

namespace devmand {
namespace channels {
namespace cli {
namespace sshsession {

using std::string;

void SshSession::close() {
  if (sessionState.channel != nullptr) {
    if (ssh_channel_is_open(sessionState.channel) != 0) {
      ssh_channel_close(sessionState.channel);
    }
    ssh_channel_free(sessionState.channel);
  }
  if (sessionState.session != nullptr) {
    if (ssh_is_connected(sessionState.session) == 1) {
      ssh_disconnect(sessionState.session);
    }
    ssh_free(sessionState.session);
  }
  sessionState.channel = nullptr;
  sessionState.session = nullptr;
}

void SshSession::openShell(
    const string& ip,
    int port,
    const string& username,
    const string& password) {
  LOG(INFO) << "Connecting to host: " << ip << " port: " << port;
  sessionState.ip = ip;
  sessionState.port = port;
  sessionState.username = username;
  sessionState.username = password;
  sessionState.session = ssh_new();
  ssh_options_set(sessionState.session, SSH_OPTIONS_HOST, ip.c_str());
  ssh_options_set(sessionState.session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
  ssh_options_set(sessionState.session, SSH_OPTIONS_PORT, &port);

  checkSuccess(ssh_connect(sessionState.session), SSH_OK);

  int rc = ssh_userauth_password(
      sessionState.session, username.c_str(), password.c_str());
  checkSuccess(rc, SSH_AUTH_SUCCESS);

  sessionState.channel = ssh_channel_new(sessionState.session);
  if (sessionState.channel == nullptr) {
    terminate();
  }

  rc = ssh_channel_open_session(sessionState.channel);
  checkSuccess(rc, SSH_OK);

  rc = ssh_channel_request_pty(sessionState.channel);
  checkSuccess(rc, SSH_OK);

  rc = ssh_channel_change_pty_size(sessionState.channel, 0, 0);
  checkSuccess(rc, SSH_OK);

  rc = ssh_channel_request_shell(sessionState.channel);
  checkSuccess(rc, SSH_OK);
}

bool SshSession::checkSuccess(int return_code, int OK_RETURN_CODE) {
  if (return_code == OK_RETURN_CODE) {
    return true;
  }
  terminate(); // TODO is this an appropriate reaction to every problem??
  return false;
}

void SshSession::terminate() {
  const char* error_message = sessionState.session != nullptr
      ? ssh_get_error(sessionState.session)
      : "unknown";
  close();
  LOG(ERROR) << "Error in SSH connection to host:  " << sessionState.ip
             << " port: " << sessionState.port;
  string error = "Error with SSH: ";
  throw std::runtime_error(error + error_message);
}

string SshSession::read(int timeoutMillis) {
  char buffer[2048];
  string result;

  while (ssh_channel_is_open(sessionState.channel) &&
         !ssh_channel_is_eof(sessionState.channel)) {
      int bytes_read;
      if (timeoutMillis == -1) {
          bytes_read = ssh_channel_read_nonblocking(
          sessionState.channel, buffer, sizeof(buffer), 0);
      } else {
          bytes_read = ssh_channel_read_timeout(
          sessionState.channel, buffer, sizeof(buffer), 0, timeoutMillis);
      }

    if (bytes_read < 0) {
      LOG(ERROR) << "Error reading data from SSH connection, read bytes: "
                 << bytes_read;
      terminate();
    } else if (bytes_read == 0) {
      return result;
    } else {
      result.append(buffer, (unsigned int)bytes_read);
    }
  }

  return "";
}

void SshSession::write(const string& command) {
  // TODO check if command is not empty
  const char* data = command.c_str();
  int bytes = ssh_channel_write(
      sessionState.channel,
      data,
      (unsigned int)command.length() * sizeof(data[0]));
  if (bytes == SSH_ERROR) {
    LOG(ERROR) << "Error while executing command " << command;
    terminate();
  }
}

SshSession::~SshSession() {
  close();
}

SshSession::SshSession(int _verbosity) : verbosity(_verbosity) {}

SshSession::SshSession() : verbosity(SSH_LOG_PROTOCOL) {}

string SshSession::readUntilOutput(string lastOutput) {
  string result;
  while (true) { //TODO this wastes CPU if output from SSH is delayed
    string output;
    if (!readQueue.pop(output) || output.empty()) {
      continue;
    }
    result.append(output);
    std::size_t found = result.find(lastOutput);
    if (found != std::string::npos) {
      // TODO check for any additional output after lastOutput
      return result.substr(0, found);
    }
  }
}

    string SshSession::read() {
        return read(-1);
    }

    socket_t SshSession::getSshFd() {
        return ssh_get_fd(sessionState.session);
    }

} // namespace sshsession
} // namespace cli
} // namespace channels
} // namespace devmand
