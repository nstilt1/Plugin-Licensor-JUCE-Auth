#include "PluginLicensorStatus.h"
PluginLicensorStatus::PluginLicensorStatus() {}

/**
 * @brief This determines whether the client will share some extra information with
 * Plugin Licensor servers. To see the data that can be sent, scroll down to readReplyFromWebserver()
 * 
 * @return true 
 * @return false 
 */
bool PluginLicensorStatus::withholdInfo ()
{
    return false;
}

/**
 * @brief This is the written language that you wrote your GUI in. You can configure
 * Plugin Licensor to send responses in whatever language that this is set to.
 * You could also have a language selection box in your software, but just remember to
 * change this method's return value.
 * 
 * The return value needs to be exactly what is typed in the Plugin Licensor console,
 * and it is untested with unicode.
 * 
 * @return juce::String 
 */
juce::String PluginLicensorStatus::getClientLanguage ()
{
    return "English";
}

/**
 * @brief This is your public key that shows for your plugin in the Plugin Licensor console
 * 
 * @return juce::RSAKey 
 */
juce::RSAKey PluginLicensorStatus::getPublicKey ()
{
    return juce::RSAKey ("5,8217cc6784348b6d29d5161414c49f130292d3f47a0840106c2d815bf474276d42dd7d0d33ffde30f4b54d49ec933565027c89adc75323120a28a70c2412054bc74b00c7800cb826e132a3272783e8c598f60b507fc256dcb16729d2ddf75b9f83669190430bffafb565b9850ec836b79f25d305622ce10e8a8f07cd18df3d600127fe4162bf3534917c366d409abec0d1252fec341b632741090c026d346d1c9e157a0020992b8c11393acd87643784585954d3bd81c9498a7a2e921311eceba29ff1fa5206cd7ecebb78d7deb1319e4f773141aa761b623550fb7c53738f1cdd6bfb9d10ecc7c9343d1c900dcd204e45334f492c6935f68d77808ff3fdd0e3");
}


/**
  * You could set this to your own URL that forwards the response to this one, if you want to
  * try to audit the usage or if you want to conceal your affiliation with Plugin Licensor.
  */
juce::URL PluginLicensorStatus::getServerAuthenticationURL ()
{
    return juce::URL ("https://4qlddpu7b6.execute-api.us-east-1.amazonaws.com/v1/auth_JUCE");
}


juce::String PluginLicensorStatus::getWebsiteName ()
{
    return "pluginlicensor.com";
}

/**
 * @brief This is your company ID in the Plugin Licensor Console
 * 
 * @return juce::String 
 */
juce::String PluginLicensorStatus::getCompanyID ()
{
    return "ALTERSHITSHO";
}

/**
 * @brief This is your product ID in the Plugin Licensor console.
 * 
 * @return juce::String 
 */
juce::String PluginLicensorStatus::getProductID ()
{
    return "MOFOF8M2P1S";
}
