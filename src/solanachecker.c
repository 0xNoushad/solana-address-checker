#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <math.h>

#define MAX_RESPONSE_SIZE 65536
#define MAX_ADDRESS_LENGTH 44
#define DEFAULT_INTERVAL 10 // seconds
#define SOLANA_RPC_URL "add your_rpc_endpoint_here" // e.g., "https://api.mainnet-beta.solana.com"
#define COINGECKO_API_URL "https://api.coingecko.com/api/v3/simple/price"
#define MAX_TOKENS 100
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_WHITE   "\x1b[37m"

// Structure to hold HTTP response
struct http_response {
    char *data;
    size_t size;
};

// Structure for token info
struct token_info {
    char mint[MAX_ADDRESS_LENGTH + 1];
    char symbol[32];
    char name[64];
    double balance;
    int decimals;
    double usd_price;
    double usd_value;
};

// Global variables
char target_address[MAX_ADDRESS_LENGTH + 1];
int monitor_interval = DEFAULT_INTERVAL;
volatile int running = 1;
double last_balance = -1; // Track balance changes
double sol_price = 0.0;
struct token_info tokens[MAX_TOKENS];
int token_count = 0;

// Well-known token mints and their symbols
struct known_token {
    const char* mint;
    const char* symbol;
    const char* name;
    const char* coingecko_id;
} known_tokens[] = {
    {"So11111111111111111111111111111111111111112", "SOL", "Solana", "solana"},
    {"EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v", "USDC", "USD Coin", "usd-coin"},
    {"Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB", "USDT", "Tether USD", "tether"},
    {"mSoLzYCxHdYgdzU16g5QSh3i5K3z3KZK7ytfqcJm7So", "mSOL", "Marinade Staked SOL", "marinade-staked-sol"},
    {"7dHbWXmci3dT8UFYWYZweBLXgycu7Y3iL6trKn1Y7ARj", "stSOL", "Lido Staked SOL", "lido-staked-sol"},
    {"DezXAZ8z7PnrnRJjz3wXBoRgixCa6xjnB7YaB1pPB263", "BONK", "Bonk", "bonk"},
    {"jtojtomepa8beP8AuQc6eXt5FriJwfFMwQx2v2f9mCL", "JTO", "Jito", "jito-governance-token"},
    {"JUPyiwrYJFskUPiHa7hkeR8VUtAeFoSYbKedZNsDvCN", "JUP", "Jupiter", "jupiter-exchange-solana"},
    {"5oVNBeEEQvYi1cX3ir8Dx5n1P7pdxydbGF2X4TxVusJm", "INF", "Infinity", ""},
    {"WENWENvqqNya429ubCdR81ZmD69brwQaaBYY6p3LCpk", "WEN", "Wen", ""},
    {NULL, NULL, NULL, NULL} // Sentinel
};

// Compatibility function for json_object_is_null (for older json-c versions)
static int json_object_is_null_compat(json_object *obj) {
    return (obj == NULL || json_object_get_type(obj) == json_type_null);
}

// Callback function to write HTTP response data
static size_t write_callback(void *contents, size_t size, size_t nmemb, struct http_response *response) {
    size_t total_size = size * nmemb;
    
    char *new_data = realloc(response->data, response->size + total_size + 1);
    if (new_data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 0;
    }
    
    response->data = new_data;
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = '\0';
    
    return total_size;
}

// Function to make RPC call to Solana
char* make_rpc_call(const char* json_data) {
    CURL *curl;
    CURLcode res;
    struct http_response response = {0};
    struct curl_slist *headers = NULL;
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\n");
        return NULL;
    }
    
    // Set headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Configure CURL
    curl_easy_setopt(curl, CURLOPT_URL, SOLANA_RPC_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SolanaMonitor/1.0");
    
    // Make the request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: CURL request failed: %s\n", curl_easy_strerror(res));
        if (response.data) {
            free(response.data);
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return response.data;
}

// Function to make HTTP GET request
char* make_http_get(const char* url) {
    CURL *curl;
    CURLcode res;
    struct http_response response = {0};
    
    curl = curl_easy_init();
    if (!curl) {
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SolanaMonitor/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        if (response.data) {
            free(response.data);
        }
        return NULL;
    }
    
    return response.data;
}

// Function to get token prices from CoinGecko
void fetch_token_prices() {
    char url[1024];
    snprintf(url, sizeof(url), 
        "%s?ids=solana,usd-coin,tether,marinade-staked-sol,lido-staked-sol,bonk,jito-governance-token,jupiter-exchange-solana&vs_currencies=usd", 
        COINGECKO_API_URL);
    
    char *response = make_http_get(url);
    if (!response) {
        printf("%sWarning: Could not fetch token prices%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }
    
    json_object *root = json_tokener_parse(response);
    if (!root) {
        free(response);
        return;
    }
    
    // Get SOL price
    json_object *solana_obj, *usd_obj;
    if (json_object_object_get_ex(root, "solana", &solana_obj) &&
        json_object_object_get_ex(solana_obj, "usd", &usd_obj)) {
        sol_price = json_object_get_double(usd_obj);
    }
    
    // Update token prices
    for (int i = 0; i < token_count; i++) {
        for (int j = 0; known_tokens[j].mint != NULL; j++) {
            if (strcmp(tokens[i].mint, known_tokens[j].mint) == 0) {
                json_object *token_obj, *price_obj;
                if (json_object_object_get_ex(root, known_tokens[j].coingecko_id, &token_obj) &&
                    json_object_object_get_ex(token_obj, "usd", &price_obj)) {
                    tokens[i].usd_price = json_object_get_double(price_obj);
                    tokens[i].usd_value = tokens[i].balance * tokens[i].usd_price;
                }
                break;
            }
        }
    }
    
    json_object_put(root);
    free(response);
}

// Function to find token info by mint
const struct known_token* find_known_token(const char* mint) {
    for (int i = 0; known_tokens[i].mint != NULL; i++) {
        if (strcmp(mint, known_tokens[i].mint) == 0) {
            return &known_tokens[i];
        }
    }
    return NULL;
}

// Function to get token accounts for an address
void get_token_accounts() {
    char json_request[512];
    snprintf(json_request, sizeof(json_request),
        "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 1,"
        "\"method\": \"getTokenAccountsByOwner\","
        "\"params\": [\"%s\", {\"programId\": \"TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA\"}, {\"encoding\": \"jsonParsed\"}]"
        "}", target_address);
    
    char *response = make_rpc_call(json_request);
    if (!response) {
        return;
    }
    
    json_object *root = json_tokener_parse(response);
    if (!root) {
        free(response);
        return;
    }
    
    json_object *result, *value;
    if (json_object_object_get_ex(root, "result", &result) &&
        json_object_object_get_ex(result, "value", &value)) {
        
        int array_len = json_object_array_length(value);
        token_count = 0;
        
        for (int i = 0; i < array_len && token_count < MAX_TOKENS; i++) {
            json_object *account_obj = json_object_array_get_idx(value, i);
            json_object *account_data, *parsed, *info, *mint, *tokenAmount, *amount, *decimals;
            
            if (json_object_object_get_ex(account_obj, "account", &account_data) &&
                json_object_object_get_ex(account_data, "data", &parsed) &&
                json_object_object_get_ex(parsed, "parsed", &parsed) &&
                json_object_object_get_ex(parsed, "info", &info) &&
                json_object_object_get_ex(info, "mint", &mint) &&
                json_object_object_get_ex(info, "tokenAmount", &tokenAmount) &&
                json_object_object_get_ex(tokenAmount, "amount", &amount) &&
                json_object_object_get_ex(tokenAmount, "decimals", &decimals)) {
                
                const char* mint_str = json_object_get_string(mint);
                double token_amount = json_object_get_double(amount);
                int token_decimals = json_object_get_int(decimals);
                
                if (token_amount > 0) {
                    strncpy(tokens[token_count].mint, mint_str, MAX_ADDRESS_LENGTH);
                    tokens[token_count].balance = token_amount / pow(10, token_decimals);
                    tokens[token_count].decimals = token_decimals;
                    tokens[token_count].usd_price = 0.0;
                    tokens[token_count].usd_value = 0.0;
                    
                    const struct known_token* known = find_known_token(mint_str);
                    if (known) {
                        strncpy(tokens[token_count].symbol, known->symbol, sizeof(tokens[token_count].symbol) - 1);
                        strncpy(tokens[token_count].name, known->name, sizeof(tokens[token_count].name) - 1);
                    } else {
                        snprintf(tokens[token_count].symbol, sizeof(tokens[token_count].symbol), "%.8s", mint_str);
                        snprintf(tokens[token_count].name, sizeof(tokens[token_count].name), "Unknown Token");
                    }
                    
                    token_count++;
                }
            }
        }
    }
    
    json_object_put(root);
    free(response);
}

// Function to format time difference in hours/minutes ago
void format_time_ago(time_t timestamp, char* buffer, size_t buffer_size) {
    if (timestamp == 0) {
        strncpy(buffer, "Unknown", buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return;
    }
    
    time_t now = time(NULL);
    double diff = difftime(now, timestamp);
    
    if (diff < 60) {
        snprintf(buffer, buffer_size, "%.0fs ago", diff);
    } else if (diff < 3600) {
        snprintf(buffer, buffer_size, "%.0fm ago", diff / 60);
    } else if (diff < 86400) {
        snprintf(buffer, buffer_size, "%.1fh ago", diff / 3600);
    } else {
        snprintf(buffer, buffer_size, "%.1fd ago", diff / 86400);
    }
}

// Function to get account balance
int get_account_balance() {
    char json_request[512];
    snprintf(json_request, sizeof(json_request),
        "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 1,"
        "\"method\": \"getBalance\","
        "\"params\": [\"%s\"]"
        "}", target_address);
    
    char *response = make_rpc_call(json_request);
    if (!response) {
        printf("%sError: No response from RPC call%s\n", COLOR_RED, COLOR_RESET);
        return 0;
    }
    
    json_object *root = json_tokener_parse(response);
    if (!root) {
        printf("%sError: Failed to parse JSON response%s\n", COLOR_RED, COLOR_RESET);
        free(response);
        return 0;
    }
    
    json_object *result;
    json_object *value;
    json_object *error;
    
    // Check for error in response
    if (json_object_object_get_ex(root, "error", &error)) {
        json_object *message;
        if (json_object_object_get_ex(error, "message", &message)) {
            printf("%sRPC Error: %s%s\n", COLOR_RED, json_object_get_string(message), COLOR_RESET);
        } else {
            printf("%sUnknown RPC Error%s\n", COLOR_RED, COLOR_RESET);
        }
        json_object_put(root);
        free(response);
        return 0;
    }
    
    if (json_object_object_get_ex(root, "result", &result) &&
        json_object_object_get_ex(result, "value", &value)) {
        
        double balance_lamports = json_object_get_double(value);
        double balance_sol = balance_lamports / 1000000000.0; // Convert lamports to SOL
        double usd_value = balance_sol * sol_price;
        
        // Check for balance changes
        if (last_balance >= 0 && last_balance != balance_sol) {
            double change = balance_sol - last_balance;
            const char* color = change > 0 ? COLOR_GREEN : COLOR_RED;
            printf("%sSOL Balance: %.6f SOL%s ($%.2f) %s[%+.6f SOL]%s\n", 
                   COLOR_CYAN, balance_sol, COLOR_RESET, usd_value, 
                   color, change, COLOR_RESET);
        } else {
            printf("%sSOL Balance: %.6f SOL%s ($%.2f)\n", 
                   COLOR_CYAN, balance_sol, COLOR_RESET, usd_value);
        }
        
        last_balance = balance_sol;
        json_object_put(root);
        free(response);
        return 1;
    } else {
        printf("%sError: Could not retrieve balance from response%s\n", COLOR_RED, COLOR_RESET);
        json_object_put(root);
        free(response);
        return 0;
    }
}

// Function to display token holdings
void display_token_holdings() {
    if (token_count == 0) {
        printf("\n%sNo token holdings found.%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }
    
    printf("\n%sToken Holdings (%d tokens):%s\n", COLOR_MAGENTA, token_count, COLOR_RESET);
    printf("%-12s %-20s %-18s %-12s %-15s\n", "Symbol", "Balance", "USD Price", "USD Value", "Name");
    printf("%s", COLOR_BLUE);
    for (int i = 0; i < 88; i++) printf("-");
    printf("%s\n", COLOR_RESET);
    
    double total_usd_value = (last_balance >= 0) ? last_balance * sol_price : 0;
    
    for (int i = 0; i < token_count; i++) {
        const char* balance_color = tokens[i].balance > 1000000 ? COLOR_GREEN : 
                                   tokens[i].balance > 1000 ? COLOR_YELLOW : COLOR_WHITE;
        const char* value_color = tokens[i].usd_value > 1000 ? COLOR_GREEN :
                                 tokens[i].usd_value > 100 ? COLOR_YELLOW : COLOR_WHITE;
        
        printf("%s%-12s%s ", COLOR_CYAN, tokens[i].symbol, COLOR_RESET);
        printf("%s%-20.6f%s ", balance_color, tokens[i].balance, COLOR_RESET);
        
        if (tokens[i].usd_price > 0) {
            printf("$%-17.6f ", tokens[i].usd_price);
            printf("%s$%-14.2f%s ", value_color, tokens[i].usd_value, COLOR_RESET);
            total_usd_value += tokens[i].usd_value;
        } else {
            printf("%-18s %-15s ", "N/A", "N/A");
        }
        
        printf("%-15s\n", tokens[i].name);
    }
    
    printf("%s", COLOR_BLUE);
    for (int i = 0; i < 88; i++) printf("-");
    printf("%s\n", COLOR_RESET);
    printf("%sTotal Portfolio Value: $%.2f%s\n", COLOR_GREEN, total_usd_value, COLOR_RESET);
}

// Function to get recent transaction signatures for an address
void get_recent_transactions() {
    char json_request[512];
    snprintf(json_request, sizeof(json_request),
        "{"
        "\"jsonrpc\": \"2.0\","
        "\"id\": 1,"
        "\"method\": \"getSignaturesForAddress\","
        "\"params\": [\"%s\", {\"limit\": 10}]"
        "}", target_address);
    
    char *response = make_rpc_call(json_request);
    if (!response) {
        printf("%sError: No response from RPC call%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    json_object *root = json_tokener_parse(response);
    if (!root) {
        printf("%sError: Failed to parse JSON response%s\n", COLOR_RED, COLOR_RESET);
        free(response);
        return;
    }
    
    json_object *result;
    json_object *error;
    
    // Check for error in response
    if (json_object_object_get_ex(root, "error", &error)) {
        json_object *message;
        if (json_object_object_get_ex(error, "message", &message)) {
            printf("%sRPC Error: %s%s\n", COLOR_RED, json_object_get_string(message), COLOR_RESET);
        } else {
            printf("%sUnknown RPC Error%s\n", COLOR_RED, COLOR_RESET);
        }
        json_object_put(root);
        free(response);
        return;
    }
    
    if (json_object_object_get_ex(root, "result", &result)) {
        int array_len = json_object_array_length(result);
        
        if (array_len == 0) {
            printf("\n%sNo recent transactions found.%s\n", COLOR_YELLOW, COLOR_RESET);
        } else {
            printf("\n%sRecent Transactions (%d found):%s\n", COLOR_MAGENTA, array_len, COLOR_RESET);
            printf("%-64s %-12s %-15s\n", "Signature", "Status", "Time");
            printf("%s", COLOR_BLUE);
            for (int i = 0; i < 96; i++) printf("-");
            printf("%s\n", COLOR_RESET);
            
            for (int i = 0; i < array_len; i++) {
                json_object *tx = json_object_array_get_idx(result, i);
                json_object *signature, *err, *block_time;
                
                if (json_object_object_get_ex(tx, "signature", &signature)) {
                    const char *sig_str = json_object_get_string(signature);
                    
                    // Get error status - using compatibility function
                    const char *status = "Success";
                    const char *status_color = COLOR_GREEN;
                    if (json_object_object_get_ex(tx, "err", &err) && !json_object_is_null_compat(err)) {
                        status = "Failed";
                        status_color = COLOR_RED;
                    }
                    
                    // Get block time and format as "X hours ago"
                    char time_str[20] = "Unknown";
                    if (json_object_object_get_ex(tx, "blockTime", &block_time) && !json_object_is_null_compat(block_time)) {
                        time_t timestamp = json_object_get_int64(block_time);
                        format_time_ago(timestamp, time_str, sizeof(time_str));
                    }
                    
                    // Truncate signature for display
                    char short_sig[65];
                    snprintf(short_sig, sizeof(short_sig), "%.64s", sig_str);
                    
                    printf("%-64s %s%-12s%s %-15s\n", 
                           short_sig, status_color, status, COLOR_RESET, time_str);
                }
            }
        }
    } else {
        printf("%sError: Could not retrieve transaction signatures%s\n", COLOR_RED, COLOR_RESET);
    }
    
    json_object_put(root);
    free(response);
}

// Signal handler for graceful shutdown
void handle_signal(int sig) {
    (void)sig; // Suppress unused parameter warning
    printf("\n%sShutting down monitor...%s\n", COLOR_YELLOW, COLOR_RESET);
    running = 0;
}

// Function to validate Solana address
int is_valid_solana_address(const char *address) {
    size_t len = strlen(address);
    if (len < 32 || len > 44) {
        return 0;
    }
    
    // Basic validation - check if it contains only valid base58 characters
    const char *valid_chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    for (size_t i = 0; i < len; i++) {
        if (strchr(valid_chars, address[i]) == NULL) {
            return 0;
        }
    }
    
    return 1;
}

// Function to print program header
void print_header() {
    printf("%s", COLOR_CYAN);
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                              ║\n");
    printf("║                           SOLANA ADDRESS MONITOR                             ║\n");
    printf("║                                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("%s", COLOR_RESET);
}

// Main monitoring function
void monitor_address() {
    print_header();
    
    printf("\n%sMonitoring Configuration:%s\n", COLOR_MAGENTA, COLOR_RESET);
    printf("Address: %s%s%s\n", COLOR_YELLOW, target_address, COLOR_RESET);
    printf("Update interval: %s%d seconds%s\n", COLOR_YELLOW, monitor_interval, COLOR_RESET);
    printf("RPC Endpoint: %s%s%s\n", COLOR_YELLOW, SOLANA_RPC_URL, COLOR_RESET);
    printf("\n%sPress Ctrl+C to stop monitoring%s\n", COLOR_GREEN, COLOR_RESET);
    
    int check_count = 0;
    
    while (running) {
        check_count++;
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_buffer[26];
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        
        printf("\n%s", COLOR_BLUE);
        for (int i = 0; i < 80; i++) printf("═");
        printf("%s\n", COLOR_RESET);
        printf("%s[Check #%d - %s] Fetching address activity...%s\n", 
               COLOR_BLUE, check_count, time_buffer, COLOR_RESET);
        
        // Fetch token prices first
        printf("%sFetching token prices...%s\n", COLOR_YELLOW, COLOR_RESET);
        fetch_token_prices();
        
        // Get current SOL balance
        if (!get_account_balance()) {
            printf("%sRetrying in %d seconds...%s\n", COLOR_YELLOW, monitor_interval, COLOR_RESET);
        } else {
            // Get token accounts
            printf("%sFetching token holdings...%s\n", COLOR_YELLOW, COLOR_RESET);
            get_token_accounts();
            
            // Display token holdings with USD values
            display_token_holdings();
            
            // Get recent transactions
            get_recent_transactions();
        }
        
        if (running) {
            printf("\n%sNext update in %d seconds... (Press Ctrl+C to stop)%s\n", 
                   COLOR_GREEN, monitor_interval, COLOR_RESET);
            
            // Wait for the specified interval (check every second for interrupt)
            for (int i = 0; i < monitor_interval && running; i++) {
                sleep(1);
            }
        }
    }
}

// Print usage information
void print_usage(const char *program_name) {
    printf("Usage: %s <solana_address> [interval_seconds]\n\n", program_name);
    printf("Arguments:\n");
    printf("  solana_address    - Valid Solana address (32-44 characters, base58 encoded)\n");
    printf("  interval_seconds  - Update interval in seconds (default: %d)\n\n", DEFAULT_INTERVAL);
    printf("Features:\n");
    printf("  • Real-time SOL balance monitoring with USD values\n");
    printf("  • Complete token portfolio with holdings and USD values\n");
    printf("  • Recent transaction history with relative timestamps\n");
    printf("  • Color-coded output for better readability\n");
    printf("  • Automatic price updates from CoinGecko API\n\n");
    printf("Examples:\n");
    printf("  %s So11111111111111111111111111111111111111112 30\n", program_name);
    printf("  %s EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v 10\n\n", program_name);
    printf("Common addresses to monitor:\n");
    printf("  So11111111111111111111111111111111111111112 (Wrapped SOL)\n");
    printf("  EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v (USDC)\n");
    printf("  Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB (USDT)\n");
    printf("  11111111111111111111111111111111 (System Program)\n");
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Validate and set target address
    if (!is_valid_solana_address(argv[1])) {
        printf("%sError: Invalid Solana address format%s\n", COLOR_RED, COLOR_RESET);
        printf("Address must be 32-44 characters long and contain only valid base58 characters\n");
        return 1;
    }
    
    strncpy(target_address, argv[1], MAX_ADDRESS_LENGTH);
    target_address[MAX_ADDRESS_LENGTH] = '\0';
    
    // Set monitoring interval if provided
    if (argc >= 3) {
        int interval = atoi(argv[2]);
        if (interval > 0 && interval <= 3600) { // Max 1 hour
            monitor_interval = interval;
        } else {
            printf("%sWarning: Invalid interval specified (must be 1-3600), using default %d seconds%s\n", 
                   COLOR_YELLOW, DEFAULT_INTERVAL, COLOR_RESET);
        }
    }
    
    // Initialize CURL
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        printf("%sError: Failed to initialize CURL%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Start monitoring
    monitor_address();
    
    // Cleanup
    curl_global_cleanup();
    printf("\n%sMonitor stopped. Thank you for using Enhanced Solana Monitor!%s\n", 
           COLOR_GREEN, COLOR_RESET);
    
    return 0;
}