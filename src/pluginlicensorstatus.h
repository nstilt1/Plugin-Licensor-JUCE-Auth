//==============================================================================
/*
  ==============================================================================

    PluginLicensorStatus.h
    Created: 27 Dec 2022 12:17:07pm
    Author:  Somedooby

  ==============================================================================
*/
#include <JuceHeader.h>
#pragma once
class PluginLicensorStatus : public juce::OnlineUnlockStatus
{
public:
    PluginLicensorStatus () = default;

    struct PLKeyFileData
    {
        juce::String licenseType, licenseCode;
        juce::Time checkUpTime;
    };

    //juce::String getProductID () override;
    //juce::String getCompanyID ();
    //bool withholdInfo ();
    //juce::String getClientLanguage ();
    //juce::RSAKey getPublicKey () override;

    bool PluginLicensorStatus::withholdInfo();
    juce::String PluginLicensorStatus::getClientLanguage();
    juce::URL PluginLicensorStatus::getServerAuthenticationURL() override;
    juce::String PluginLicensorStatus::getWebsiteName() override;
    juce::String PluginLicensorStatus::getCompanyID();
    juce::String PluginLicensorStatus::getProductID() override;
    juce::RSAKey PluginLicensorStatus::getPublicKey() override;
    

    /**
     * @brief Copied from OnlineUnlockStatus.cpp
     * 
     * @param hexData 
     * @param rsaPublicKey 
     * @return juce::XmlElement 
     */
    static juce::XmlElement decryptXML (juce::String hexData, juce::RSAKey rsaPublicKey)
    {
        juce::BigInteger val;
        val.parseString (hexData, 16);

        juce::RSAKey key (rsaPublicKey);
        jassert (key.isValid ());

        std::unique_ptr<juce::XmlElement> xml;

        if (!val.isZero ())
        {
            key.applyToValue (val);

            auto mb = val.toMemoryBlock ();

            if (juce::CharPointer_UTF8::isValidString (static_cast<const char*> (mb.getData ()), (int)mb.getSize ()))
                xml = parseXML (mb.toString ());
        }

        return xml != nullptr ? *xml : juce::XmlElement ("key");
    }

    /**
     * @brief Get the Xml From Key File object
     * Copied from OnlineUnlockStatus.cpp
     * @param keyFileText 
     * @param rsaPublicKey 
     * @return juce::XmlElement 
     */
    static juce::XmlElement getXmlFromKeyFile (juce::String keyFileText, juce::RSAKey rsaPublicKey)
    {
        return decryptXML (keyFileText.fromLastOccurrenceOf ("#", false, false).trim (), rsaPublicKey);
    }

    /**
     * @brief Get the License Info object
     * checkUpTime: holds the time that the client should check back with the server to
     * verify that the license is still valid
     * @return PLKeyFileData 
     */
    PLKeyFileData getLicenseInfo ()
    {
        juce::MemoryBlock mb;
        mb.fromBase64Encoding (getState ());

        if (!mb.isEmpty ())
            status = juce::ValueTree::readFromGZIPData (mb.getData (), mb.getSize ());
        else
            status = juce::ValueTree ("REG");

        juce::StringArray localMachineNums (getLocalMachineIDs ());

        PLKeyFileData data;
        juce::XmlElement xml = getXmlFromKeyFile (status["key"], getPublicKey ());
        data.checkUpTime = juce::Time (xml.getStringAttribute ("check").getHexValue64 ());
        data.licenseType = xml.getStringAttribute ("licenseType");


        return data;
    }

    /**
      * A somewhat nasty-looking method that silently checks with the server
      * to ensure that the license is still valid.
      * The info.CheckUpTime is when the cllient should check back with the server to
      * see if the license is still valid
      * It is an inline var to make it slightly harder to remove every instance,
      * similar to how JUCE implemented isUnlcked()
      */
    inline juce::var checkUp ()
    {
        load ();

        if (isUnlocked ())
        {
            auto info = getLicenseInfo ();
            if (info.checkUpTime.toMilliseconds () < juce::Time::currentTimeMillis ())
            {
                auto reply = readReplyFromWebserver (info.licenseCode, "");
                if (auto xml = parseXML (reply))
                {
                    if (auto keyNode = xml->getChildByName ("KEY"))
                    {
                        const juce::String keyText (keyNode->getAllSubText ().trim ());
                        if (keyText.length () > 10 && applyKeyFile (keyText))
                        {
                            if (xml->hasTagName ("MESSAGE"))
                            {
                                load ();
                                return isUnlocked ();
                            }
                        }
                        return isUnlocked ();
                    }
                    return isUnlocked ();
                }
                else
                {
                    return isUnlocked ();
                }
            }
            return isUnlocked ();
        }
        return isUnlocked ();
    }




    juce::StringArray getLocalMachineIDs () override
    {
        return juce::StringArray (juce::SystemStats::getUniqueDeviceID ());
    }


    



    bool PluginLicensorStatus::doesProductIDMatch (const juce::String& returnedIDFromServer) override
    {
        return getProductID () == returnedIDFromServer;
    }


    //juce::String PluginLicensorStatus::readReplyFromWebserver (const juce::String& licenseCode, const juce::String&) override;

    //void PluginLicensorStatus::userCancelled () override;





    

    
    
    

    /** 
     ======================================================================
     * @brief These functions can withhold information from the server,
     * depending on if withholdInfo() returns true. If you decide to set
     * it to true, you will probably need to inform your users that you
     * collect some data from them and share it with Plugin Licensor's 
     * community. Only anonymous information will be public, such as CPU
     * information, OS, SIMD, language and locality. For all of the gathered
     * info, check readReplyFromWebserver().
     * 
     * @param value 
     * @return juce::String 
     */
    juce::String getPrivateInt (int value)
    {
        if (withholdInfo ())
        {
            return "x";
        }
        return juce::String (value);
    }

    juce::String getPrivateBool (bool value)
    {
        if (withholdInfo ()) { return "x"; }
        return value ? "1" : "0";
    }

    juce::String getPrivateString (juce::String value)
    {
        if (withholdInfo ()) { return "x"; }
        return value;
    }
    //=====================================================================

    juce::String PluginLicensorStatus::readReplyFromWebserver (const juce::String& licenseCode, const juce::String&)
    {
        
        // It might need to be instantiated or something. I was getting an error about OnlineUnlockStatus's
        // private member `status` not being initialized
        //load ();
        juce::URL url (getServerAuthenticationURL ()
            .withParameter ("company", getCompanyID ())
            .withParameter ("product", getProductID ())
            .withParameter ("license_code", licenseCode)
            .withParameter ("mach", juce::SystemStats::getUniqueDeviceID ())
            .withParameter ("os", juce::SystemStats::getOperatingSystemName ())
            .withParameter ("is_64", juce::SystemStats::isOperatingSystem64Bit () == true ? "1" : "0")
            .withParameter ("comp_name", getPrivateString(juce::SystemStats::getComputerName ()))
            .withParameter ("logon_name", getPrivateString(juce::SystemStats::getLogonName ()))
            .withParameter ("user_region", getPrivateString (juce::SystemStats::getUserRegion ()))
            .withParameter ("display_language", juce::SystemStats::getDisplayLanguage ())
            .withParameter ("logical_cores", getPrivateInt (juce::SystemStats::getNumCpus ()))
            .withParameter ("physical_cores", "" + getPrivateInt(juce::SystemStats::getNumPhysicalCpus ()))
            .withParameter ("cpu_freq", getPrivateInt (juce::SystemStats::getCpuSpeedInMegahertz ()))
            .withParameter ("cpu_vendor", getPrivateString (juce::SystemStats::getCpuVendor ()))
            .withParameter ("cpu_model", getPrivateString (juce::SystemStats::getCpuModel ()))
            .withParameter ("memory", getPrivateInt (juce::SystemStats::getMemorySizeInMegabytes ()))
            .withParameter ("has_mmx", getPrivateBool (juce::SystemStats::hasMMX ()))
            .withParameter ("has_3DNow", getPrivateBool (juce::SystemStats::has3DNow ()))
            .withParameter ("has_FMA3", getPrivateBool (juce::SystemStats::hasFMA3 ()))
            .withParameter ("has_FMA4", getPrivateBool (juce::SystemStats::hasFMA4 ()))
            .withParameter ("has_SSE", getPrivateBool (juce::SystemStats::hasSSE ()))
            .withParameter ("has_SSE2", getPrivateBool (juce::SystemStats::hasSSE2 ()))
            .withParameter ("has_SSE3", getPrivateBool (juce::SystemStats::hasSSE3 ()))
            .withParameter ("has_SSSE3", getPrivateBool (juce::SystemStats::hasSSSE3 ()))
            .withParameter ("has_SSE41", getPrivateBool (juce::SystemStats::hasSSE41 ()))
            .withParameter ("has_SSE42", getPrivateBool (juce::SystemStats::hasSSE42 ()))
            .withParameter ("has_AVX", getPrivateBool (juce::SystemStats::hasAVX ()))
            .withParameter ("has_AVX2", getPrivateBool (juce::SystemStats::hasAVX2 ()))
            .withParameter ("has_AVX512F", getPrivateBool (juce::SystemStats::hasAVX512F ()))
            .withParameter ("has_AVX512BW", getPrivateBool (juce::SystemStats::hasAVX512BW ()))
            .withParameter ("has_AVX512CD", getPrivateBool (juce::SystemStats::hasAVX512CD ()))
            .withParameter ("has_AVX512DQ", getPrivateBool (juce::SystemStats::hasAVX512DQ ()))
            .withParameter ("has_AVX512ER", getPrivateBool (juce::SystemStats::hasAVX512ER ()))
            .withParameter ("has_AVX512IFMA", getPrivateBool (juce::SystemStats::hasAVX512IFMA ()))
            .withParameter ("has_AVX512PF", getPrivateBool (juce::SystemStats::hasAVX512PF ()))
            .withParameter ("has_AVX512VBMI", getPrivateBool (juce::SystemStats::hasAVX512VBMI ()))
            .withParameter ("has_AVX512VL", getPrivateBool (juce::SystemStats::hasAVX512VL ()))
            .withParameter ("has_AVX512VPOPCNTDQ", getPrivateBool (juce::SystemStats::hasAVX512VPOPCNTDQ ()))
            .withParameter ("has_Neon", getPrivateBool (juce::SystemStats::hasNeon ()))
            .withParameter ("client_language", getClientLanguage ())


        );

        // I don't know why the following code seems to be necessary. I originally configured 
        // this to only work with a POST request body, but it didn't work, so I enabled 
        // query string parameters, which took a while, but now it gets a valid response.

        auto paramNames = url.getParameterNames ();
        auto paramVals = url.getParameterValues ();

        juce::DynamicObject* obj = new juce::DynamicObject ();
        juce::String body = "";
        for (int i = 0; i < paramNames.size (); ++i)
        {
            //body = appendJson (body, paramNames[i], paramVals[i]);
            obj->setProperty (paramNames[i], paramVals[i]);
        }
        juce::var json (obj);
        juce::String s = juce::JSON::toString (json);
        url = url.withPOSTData (s);

        /*/
        body = endJson (body);

        url = url.withPOSTData ("body:" + body);
        DBG(url.getPostData ());
        /*/
        //url.createInputStream()

        std::unique_ptr<juce::WebInputStream> stream;

        DBG ("Trying to unlock via URL: " << url.toString (true));

        {
            //juce::CriticalSection streamCreationLock2;
            juce::ScopedLock lock (streamCreationLock);
            stream.reset (new juce::WebInputStream (url, false));
        }

        if (stream->connect (nullptr))
        {
            auto* thread = juce::Thread::getCurrentThread ();

            if (thread->threadShouldExit () || stream->isError ())
                return {};

            auto contentLength = stream->getTotalLength ();
            auto downloaded = 0;

            const size_t bufferSize = 0x8000;
            juce::HeapBlock<char> buffer (bufferSize);

            while (!(stream->isExhausted () || stream->isError () || thread->threadShouldExit ()))
            {
                auto max = juce::jmin ((int)bufferSize, contentLength < 0 ? std::numeric_limits<int>::max ()
                    : static_cast<int> (contentLength - downloaded));

                auto actualBytesRead = stream->read (buffer.get () + downloaded, max - downloaded);

                if (actualBytesRead < 0 || thread->threadShouldExit () || stream->isError ())
                    break;

                downloaded += actualBytesRead;

                if (downloaded == contentLength)
                    break;
            }

            if (thread->threadShouldExit () || stream->isError () || (contentLength > 0 && downloaded < contentLength))
                return {};

            save ();
            return { juce::CharPointer_UTF8 (buffer.get ()) };
        }

        return {};
    }

    void PluginLicensorStatus::userCancelled ()
    {
        juce::ScopedLock lock (streamCreationLock);

        if (stream != nullptr)
            stream->cancel ();
    }



    /*/
        // This will probably be used to store the encrypted license
        // data in an image in the binary data. Anyone is welcome to
        // try to get it to work, but it isn't necessary
        // I'm not entirely sure how to do that, or if it is allowed by
        // some strict operating systems for software to add data to itself

        juce::PixelARGB setPixel (juce::String sub, int end)
        {
            auto pixelVals = juce::Array<juce::uint8> (255, 255, 255, 255);
            for (int i = 0; i < 3; ++i)
            {
                auto sub2 = sub.substring (i * 2, i * 2 + 2);
                if (sub2.length () > 0)
                {
                    juce::uint8 val = strtol (sub2.toRawUTF8 (), NULL, 16);
                    pixelVals.set (i, val);
                }
                else
                {
                    pixelVals.set (i, 255);
                }

            }

            if (end <= 0)
            {
                pixelVals.set (3, 6 - abs(end));
            }
            else
            {
                pixelVals.set (3, 255);
            }
            return juce::PixelARGB (pixelVals[0], pixelVals[1], pixelVals[2], pixelVals[3]);
        }
    /*/


private:
    juce::CriticalSection streamCreationLock;
    std::unique_ptr<juce::WebInputStream> stream;
    juce::ValueTree status;
    juce::ApplicationProperties props;

};