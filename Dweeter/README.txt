Dweeter4.ino: uses Wifi.begin() approach

cleaned up code. Use this as the basline for simplest form of :
 - Wifi using Wifi.begin(ssid,pwd)
 - POST using HTTPClient
 
This sends the number of packet failures also to Dweet/jsonplaceholder
failures at various stages of the flow are separately coded. 

------------------------------------------------------------
Dweeter6.ino: uses WifiMulti
uses WifiMulti with explicit code to reconnect every time Wifi fails

observations:
At the startup, it scans and connects to the best AP
Subsequently the best AP fails.
It attempts to reconnect to that *same* best AP.
Even if some other (even stronger) AP comes up later, it does not try to connect to it.

If initial attempt itself fails, it never recovers from it. Even if all 3 APs come up later.

Solution to both the above problems: 
    call wifi_multi_client.run()  every 5 seconds or when the next packet is due.
------------------------------------------------------------    