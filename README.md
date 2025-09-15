# Solana Address Monitor

A real-time command-line tool for monitoring Solana addresses with comprehensive portfolio tracking, transaction history, and USD value calculations.

## Features

✨ **Real-time SOL balance monitoring** with USD values  
🪙 **Complete token portfolio** with holdings and USD values  
📊 **Recent transaction history** with relative timestamps  
🎨 **Color-coded output** for better readability  
💰 **Automatic price updates** from CoinGecko API  
⚡ **Balance change notifications** with difference highlighting  
🔄 **Configurable update intervals**  

## Screenshots

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║                           SOLANA ADDRESS MONITOR                             ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝

SOL Balance: 15.234567 SOL ($2,341.23) [+0.001234 SOL]

Token Holdings (5 tokens):
Symbol       Balance              USD Price          USD Value       Name           
────────────────────────────────────────────────────────────────────────────────────
USDC         1,250.000000         $1.000000          $1,250.00       USD Coin       
BONK         45,678,901.234567    $0.000012          $548.15         Bonk           
JUP          123.456789           $1.250000          $154.32         Jupiter        
mSOL         5.123456             $168.450000        $863.21         Marinade SOL   
USDT         750.000000           $1.000000          $750.00         Tether USD     
────────────────────────────────────────────────────────────────────────────────────
Total Portfolio Value: $5,906.91
```

## Prerequisites

Before installing, make sure you have the following dependencies installed on your system:

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev libjson-c-dev pkg-config
```

### CentOS/RHEL/Fedora:
```bash
# CentOS/RHEL
sudo yum install gcc gcc-c++ make libcurl-devel json-c-devel pkgconfig

# Fedora
sudo dnf install gcc gcc-c++ make libcurl-devel json-c-devel pkgconfig
```

### macOS:
```bash
# Install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install curl json-c pkg-config
```

### Arch Linux:
```bash
sudo pacman -S base-devel curl json-c pkg-config
```

## Installation

### Method 1: Download and Compile

1. **Download the source code:**
   ```bash
   wget https://path-to-your-source/solana_monitor.c
   # or
   curl -O https://path-to-your-source/solana_monitor.c
   ```

2. **Compile the program:**
   ```bash
   gcc -o solana_monitor solana_monitor.c -lcurl -ljson-c -lm
   ```

3. **Make it executable:**
   ```bash
   chmod +x solana_monitor
   ```

### Method 2: Clone from Repository

```bash
git clone https://github.com/your-username/solana-monitor.git
cd solana-monitor
make
```

### Method 3: One-liner Installation

```bash
curl -O https://path-to-your-source/solana_monitor.c && gcc -o solana_monitor solana_monitor.c -lcurl -ljson-c -lm && chmod +x solana_monitor
```

## Configuration

### 1. Set up Solana RPC Endpoint

Before running the monitor, you need to configure your Solana RPC endpoint:

1. **Edit the source code** and replace `add your_rpc_endpoint_here` with your RPC URL:
   ```c
   #define SOLANA_RPC_URL "https://api.mainnet-beta.solana.com"
   ```

2. **Recommended RPC Providers:**
   - **Free (with rate limits):**
     - `https://api.mainnet-beta.solana.com` (Official Solana RPC)
   - **Premium (better performance):**
     - [Helius](https://helius.xyz): `https://rpc.helius.xyz/?api-key=YOUR_KEY`
     - [QuickNode](https://quicknode.com): `https://your-endpoint.solana-mainnet.quiknode.pro/YOUR_KEY/`
     - [Alchemy](https://alchemy.com): `https://solana-mainnet.g.alchemy.com/v2/YOUR_KEY`

3. **Recompile after changing the RPC URL:**
   ```bash
   gcc -o solana_monitor solana_monitor.c -lcurl -ljson-c -lm
   ```

## Usage

### Basic Usage

```bash
./solana_monitor <solana_address> [interval_seconds]
```

### Parameters

- `solana_address` - Valid Solana address (32-44 characters, base58 encoded)
- `interval_seconds` - Optional: Update interval in seconds (default: 10, max: 3600)

### Examples

1. **Monitor a Solana address with default 10-second intervals:**
   ```bash
   ./solana_monitor So11111111111111111111111111111111111111112
   ```

2. **Monitor with custom 30-second intervals:**
   ```bash
   ./solana_monitor EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v 30
   ```

3. **Monitor a wallet address:**
   ```bash
   ./solana_monitor 9WzDXwBbmkg8ZTbNMqUxvQRAyrZzDsGYdLVL9zYtAWWM 15
   ```

### Common Addresses to Monitor

- **Wrapped SOL**: `So11111111111111111111111111111111111111112`
- **USDC**: `EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v`
- **USDT**: `Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB`
- **System Program**: `11111111111111111111111111111111`

## Supported Tokens

The monitor includes built-in support for popular Solana tokens with automatic price fetching:

| Symbol | Name | CoinGecko Integration |
|--------|------|---------------------|
| SOL | Solana | ✅ |
| USDC | USD Coin | ✅ |
| USDT | Tether USD | ✅ |
| mSOL | Marinade Staked SOL | ✅ |
| stSOL | Lido Staked SOL | ✅ |
| BONK | Bonk | ✅ |
| JTO | Jito | ✅ |
| JUP | Jupiter | ✅ |
| WEN | Wen | ⏳ |
| INF | Infinity | ⏳ |

*Unknown tokens will be displayed with truncated mint addresses*

## Troubleshooting

### Common Issues

1. **"Error: Failed to initialize CURL"**
   ```bash
   # Install libcurl development libraries
   sudo apt install libcurl4-openssl-dev  # Ubuntu/Debian
   sudo yum install libcurl-devel          # CentOS/RHEL
   ```

2. **"Error: No response from RPC call"**
   - Check your internet connection
   - Verify your RPC endpoint is correct and accessible
   - Some RPC endpoints may have rate limits

3. **"Error: Invalid Solana address format"**
   - Ensure the address is 32-44 characters long
   - Verify it contains only valid base58 characters (no 0, O, I, l)

4. **"undefined reference to json_object_*"**
   ```bash
   # Install json-c development libraries
   sudo apt install libjson-c-dev  # Ubuntu/Debian
   sudo yum install json-c-devel   # CentOS/RHEL
   ```

5. **Compilation errors with older systems:**
   ```bash
   # Try compiling with specific flags
   gcc -o solana_monitor solana_monitor.c -lcurl -ljson-c -lm -std=c99
   ```

### Performance Tips

1. **Use a premium RPC provider** for better reliability and faster responses
2. **Increase monitoring interval** (30-60 seconds) to reduce API calls
3. **Monitor specific token accounts** instead of large wallet addresses for faster queries

## Advanced Usage

### Running as a Service (Linux)

1. **Create a service file:**
   ```bash
   sudo nano /etc/systemd/system/solana-monitor.service
   ```

2. **Add service configuration:**
   ```ini
   [Unit]
   Description=Solana Address Monitor
   After=network.target

   [Service]
   Type=simple
   User=your-username
   WorkingDirectory=/path/to/solana-monitor
   ExecStart=/path/to/solana-monitor/solana_monitor YOUR_ADDRESS 30
   Restart=always
   RestartSec=5

   [Install]
   WantedBy=multi-user.target
   ```

3. **Enable and start the service:**
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable solana-monitor
   sudo systemctl start solana-monitor
   ```

### Logging Output

To save monitoring output to a file:
```bash
./solana_monitor YOUR_ADDRESS 30 | tee -a monitor.log
```

To run in background:
```bash
nohup ./solana_monitor YOUR_ADDRESS 30 > monitor.log 2>&1 &
```

## Security Notes

- The monitor only **reads** blockchain data and never accesses private keys
- RPC endpoints may log IP addresses and requested data
- Use reputable RPC providers to ensure data privacy
- Be cautious when monitoring addresses in public/shared environments

## Contributing

Contributions are welcome! Areas for improvement:

- Additional token support
- Transaction parsing and categorization
- Web interface
- Database logging
- Alert notifications
- Multi-address monitoring

## License

This project is open source. Please check the license file for specific terms.

## Support

For issues and questions:
1. Check the troubleshooting section above
2. Verify your system meets all prerequisites
3. Test with a known working Solana address
4. Create an issue with your system details and error messages

## Changelog

### v1.0.0
- Initial release
- Real-time balance monitoring
- Token portfolio tracking
- Transaction history
- USD price integration
- Color-coded terminal output