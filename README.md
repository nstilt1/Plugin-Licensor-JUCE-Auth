# Plugin-Licensor-JUCE-Auth
C++ client side library for verifying license codes with Plugin Licensor

# This needs testing

The `getStateAndSaveState` code might not be required, but I don't know for sure. I don't even know if it needs to be implemented.

I have not gotten the client-side code to work, but the back end is working, and `readReplyFromWebserver()` appears to work with the parameters that are in the code. My code is somewhat documented enough so that you can understand what I'm trying to do with it, but I will restate part of it here.

## The Server Response
* `check` value
  * The server, on success, will respond in the same format that JUCE uses, but with 2 extra details. Inside the encrypted key data is a `check` value, that tells when the client should check back with the server to verify that the license is still valid. It will do this for all license types, even non-expiring licenses. This is to prevent offline licenses from being cracked as easily, but no client-side software is immune from cracking. 

  * This `check` value will also allow for users to deactivate machines on their licenses from your website. That part isn't implemented yet, but it will be first available for Wordpress sites. It will probably also allow them to regenerate their license code, removing all `online` machines from the license. I could make it attempt to remove `offline` machines too, but those licenses still don't expire if they only use the software offline... so I might give developers an option about how that functionality works.

* `licenseType` value
  * The server also responds with a `licenseType` value inside the encrypted key data. If you want to make your Trial mode have less functionalities, you will be able to determine if the user is on the Trial license.

## Something to look out for
There is a potential issue with the `check` value. If the user is not connected to the internet when they open their unlocked plugin, and if it sends a request to the server but can't connect, then it should not deactivate their license... but it might. So that needs to be tested.
