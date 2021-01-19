#include "ReceiverCli.hpp"

#include "devices.hpp"
#include "utilities/print/Print.hpp"
#include "utilities/strings/Strings.hpp"

using namespace Cli;

static void readProbes(uint16_t argc, ArgV argv);

static Command commands[] =
{
    {.name = "READ", .function = &readProbes}
};

static CommandInterface cli(pUart, commands, 1);
CommandInterface* pCli = &cli;

static void readProbes(uint16_t argc, ArgV argv)
{
    if (argc < 2)
    {
        // Incorrect number of parameters
        PRINTLN("Incorrect # of params");
        return;
    }

    uint8_t probeId = (uint8_t)Strings::str2int(argv[1]);
    pVeranusReceiver->getUpdate(probeId);
}