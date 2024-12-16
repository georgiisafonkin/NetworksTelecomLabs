import asyncio
import struct
import sys

#
# Configuration
#
BUFSIZE = 2048
LOCAL_ADDR = '0.0.0.0'
LOCAL_PORT = int(sys.argv[1])
OUTGOING_INTERFACE = ""  # Set interface if needed

#
# Constants
#
VER = b'\x05'
M_NOAUTH = b'\x00'
M_NOTAVAILABLE = b'\xff'
CMD_CONNECT = b'\x01'
ATYP_IPV4 = b'\x01'
ATYP_DOMAINNAME = b'\x03'


async def proxy_data(reader_src, writer_src, reader_dst, writer_dst):
    """Transfer data between source and destination asynchronously."""
    try:
        while True:
            data = await reader_src.read(BUFSIZE)
            if not data:
                break
            writer_dst.write(data)
            await writer_dst.drain()
    except asyncio.CancelledError:
        pass  # Task was canceled, terminate the loop gracefully.
    except Exception as e:
        print(f"Error during proxying: {e}")


async def connect_to_dst(dst_addr, dst_port):
    """Connect to the target destination asynchronously."""
    try:
        reader, writer = await asyncio.open_connection(dst_addr, dst_port)
        return reader, writer
    except Exception as e:
        print(f"Failed to connect to {dst_addr}:{dst_port} - {e}")
        return None, None


async def handle_client(reader, writer):
    """Handle SOCKS5 client connection."""
    try:
        # Subnegotiation
        identification_packet = await reader.read(BUFSIZE)
        if identification_packet[:1] != VER:
            writer.close()
            await writer.wait_closed()
            return

        nmethods = identification_packet[1]
        methods = identification_packet[2:2 + nmethods]
        if M_NOAUTH not in methods:
            writer.write(VER + M_NOTAVAILABLE)
            await writer.drain()
            writer.close()
            await writer.wait_closed()
            return

        writer.write(VER + M_NOAUTH)
        await writer.drain()

        # Request
        request_packet = await reader.read(BUFSIZE)
        if request_packet[:3] != VER + CMD_CONNECT + b'\x00':
            writer.close()
            await writer.wait_closed()
            return

        if request_packet[3:4] == ATYP_IPV4:
            dst_addr = socket.inet_ntoa(request_packet[4:8])
            dst_port = struct.unpack(">H", request_packet[8:10])[0]
        elif request_packet[3:4] == ATYP_DOMAINNAME:
            domain_len = request_packet[4]
            dst_addr = request_packet[5:5 + domain_len].decode()
            dst_port = struct.unpack(">H", request_packet[5 + domain_len:7 + domain_len])[0]
        else:
            writer.close()
            await writer.wait_closed()
            return

        # Connect to destination
        dst_reader, dst_writer = await connect_to_dst(dst_addr, dst_port)
        if not dst_reader or not dst_writer:
            writer.write(VER + b'\x01' + b'\x00' + ATYP_IPV4 + b'\x00' * 6)
            await writer.drain()
            writer.close()
            await writer.wait_closed()
            return

        bnd_addr, bnd_port = dst_writer.get_extra_info('sockname')
        bnd_addr = struct.pack('>4B', *map(int, bnd_addr.split('.')))
        bnd_port = struct.pack('>H', bnd_port)

        writer.write(VER + b'\x00' + b'\x00' + ATYP_IPV4 + bnd_addr + bnd_port)
        await writer.drain()

        # Start proxying data
        client_to_server = asyncio.create_task(proxy_data(reader, writer, dst_reader, dst_writer))
        server_to_client = asyncio.create_task(proxy_data(dst_reader, dst_writer, reader, writer))

        await asyncio.wait([client_to_server, server_to_client], return_when=asyncio.FIRST_COMPLETED)
    except Exception as e:
        print(f"Error handling client: {e}")
    finally:
        writer.close()
        await writer.wait_closed()


async def main():
    """Start the asynchronous SOCKS5 proxy server."""
    server = await asyncio.start_server(handle_client, LOCAL_ADDR, LOCAL_PORT)
    addr = server.sockets[0].getsockname()
    print(f"Serving on {addr}")

    async with server:
        await server.serve_forever()


if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server shutting down.")
