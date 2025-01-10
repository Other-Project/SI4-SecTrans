# SecTrans

## Group 9 

* **Falcoz** Alban 
* **Galli** Evan 
* **Gripari** Alexandre 
* **Lassauniere** Theo

# Potential Vulnerabilities

## Network Vulnerabilities

### Man-in-the-Middle

Man-in-the-Middle attacks represent a major threat when communication lacks proper encryption. Attackers can intercept or alter transmitted data, compromising confidentiality and integrity. To mitigate this risk, it is crucial to ensure that all communication channels are encrypted.

### Replay attacks

Another vulnerability in the network is replay attacks, where an attacker intercepts legitimate requests and repeats them to perform unauthorised actions. These attacks can be effectively countered by adding nonces to all requests to ensure their uniqueness and prevent their reuse.

## Server Vulnerabilities

### Unauthorized Access

Servers are vulnerable to unauthorised access if appropriate authentication mechanisms and access controls are not in place. This can lead to malicious actors gaining access to sensitive files and resources. Implementing robust authentication methods and fine-grained access controls is essential to prevent such breaches.

### File injection

File injection attacks are another common threat to servers. Attackers can try to access unwanted files using relative paths in the filename. To mitigate this risk, it is necessary to validate and sanitize all file names rigorously, while enforcing strict restrictions on file types and sizes.

### Microhard Vulnerabilities

We've done retro-engineering of the Microhard library to better understand how it work and find potentials vulnerabilities. 


We discovered a possibility of buffer overflow in it, caused by reading incoming message sizes without validating them. Indeed, in the following code snippet, a malicious client could trigger a buffer overflow by providing an excessively large size:
```c
ssize_t valread = read(new_socket, buf, 5);
v4 = atoi((const char *)buf);
valread = read(new_socket, buf, v4);
```
This vulnerability can't be patched without recompiling the library.

Another vulnerability arises from unsafe string copy operations, where the server copies messages to a buffer without checking size constraints, as shown below:

   ```c
   strcpy(a1, (const char *)buf);
   ```
  
Replacing unsafe functions like strcpy with safer alternatives such as strncpy and enforcing strict buffer size checks can effectively mitigate this risk.


## Attack Tree

The attack tree below represents potential attack scenarios.

1. **Intercept Communication**
   - Perform a Man-in-the-Middle attack to eavesdrop on data

2. **Exploit the Server**
   - Upload malicious files
   - Replay valid requests
   - Replay unvalid requets (fuzzing)

3. **Exploit the Client** 
   - Inject malicious input (fuzzing)
