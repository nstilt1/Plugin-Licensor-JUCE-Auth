//==============================================================================
/*
  ==============================================================================

    PluginLicensorStatus.h
    Created: 27 Dec 2022 12:17:07pm
    Author:  Somedooby

  ==============================================================================
*/

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

    void saveState (const juce::String& data) override
    {
        //
        auto options = juce::PropertiesFile::Options ();
        options.applicationName = ("Plugin Licensor " + getProductID ());
        options.filenameSuffix = ".license";
        options.folderName = "Plugin Licensor";
        options.osxLibrarySubFolder = "Application Support/Plugin Licensor";

        props.setStorageParameters (options);
        auto xml = juce::XmlElement ("key");
        xml.setAttribute ("key", data);
        props.getUserSettings ()->setValue (getProductID (), &xml);
        auto r = props.getUserSettings ()->save();
        if (!r)
        {
            DBG ("Couldn't save");
        }
        /*/

        //
        juce::String userAppDir = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory).getFullPathName ();
        juce::File parentDir = juce::File (userAppDir + "/Plugin Licensor/" + getCompanyID () + "/");
        if (!parentDir.exists ())
        {
            parentDir.createDirectory ();
        }
        juce::File licenseFile = juce::File (parentDir.getFullPathName () + getProductID () + ".PLlicense");
        if (licenseFile.exists ())
        {
            licenseFile.replaceWithText (data);
        }
        else
        {
            licenseFile.create ();
            licenseFile.replaceWithText (data);
        }
        DBG (licenseFile.getFullPathName ());
        /*/
    }

    juce::String getState () override
    {
        //
        auto options = juce::PropertiesFile::Options ();
        options.applicationName = (getProductID ());
        options.filenameSuffix = ".license";
        options.folderName = "Plugin Licensor";
        options.osxLibrarySubFolder = "Application Support/Plugin Licensor";
        
        props.setStorageParameters (options);
        if (props.getUserSettings ()->isValidFile ())
        {
            
            return juce::parseXML (props.getUserSettings ()->getFile ()).get()->getAttributeValue(0);
            
        }
        return {};
        //

        /*/
        juce::String userAppDir = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userApplicationDataDirectory).getFullPathName ();
        juce::File parentDir = juce::File (userAppDir + "/Plugin Licensor/" + getCompanyID () + "/");
        if (parentDir.exists ())
        {
            parentDir.createDirectory ();

            juce::File licenseFile = juce::File (parentDir.getFullPathName () + getProductID () + ".PLlicense");
            if (licenseFile.exists ())
            {
                return licenseFile.loadFileAsString ();
            }
            DBG (licenseFile.getFullPathName ());
        }
        return "";
        /*/
        return {};
    }

    juce::URL PluginLicensorStatus::getServerAuthenticationURL () override
    {
        return juce::URL ("https://4qlddpu7b6.execute-api.us-east-1.amazonaws.com/v1/auth_JUCE");
    }

    juce::String PluginLicensorStatus::getWebsiteName () override
    {
        return "pluginlicensor.com";
    }

    bool PluginLicensorStatus::doesProductIDMatch (const juce::String& returnedIDFromServer) override
    {
        return getProductID () == returnedIDFromServer;
    }


    //juce::String PluginLicensorStatus::readReplyFromWebserver (const juce::String& licenseCode, const juce::String&) override;

    //void PluginLicensorStatus::userCancelled () override;

    juce::String PluginLicensorStatus::getCompanyID ()
    {
        return "ALTERSHITSHO";
    }
    juce::String PluginLicensorStatus::getProductID () override
    {
        return "MOFOF8M2P1S";
    }
    bool PluginLicensorStatus::witholdInfo ()
    {
        return false;
    }
    juce::String PluginLicensorStatus::getClientLanguage ()
    {
        return "English";
    }
    juce::RSAKey PluginLicensorStatus::getPublicKey () override
    {
        return juce::RSAKey ("5,8217cc6784348b6d29d5161414c49f130292d3f47a0840106c2d815bf474276d42dd7d0d33ffde30f4b54d49ec933565027c89adc75323120a28a70c2412054bc74b00c7800cb826e132a3272783e8c598f60b507fc256dcb16729d2ddf75b9f83669190430bffafb565b9850ec836b79f25d305622ce10e8a8f07cd18df3d600127fe4162bf3534917c366d409abec0d1252fec341b632741090c026d346d1c9e157a0020992b8c11393acd87643784585954d3bd81c9498a7a2e921311eceba29ff1fa5206cd7ecebb78d7deb1319e4f773141aa761b623550fb7c53738f1cdd6bfb9d10ecc7c9343d1c900dcd204e45334f492c6935f68d77808ff3fdd0e3");
    }

    juce::String PluginLicensorStatus::readReplyFromWebserver (const juce::String& licenseCode, const juce::String&)
    {
        
        load ();
        juce::URL url (getServerAuthenticationURL ()
            .withParameter ("company", getCompanyID ())
            .withParameter ("product", getProductID ())
            .withParameter ("license_code", licenseCode)
            .withParameter ("mach", juce::SystemStats::getUniqueDeviceID ())
            .withParameter ("os", juce::SystemStats::getOperatingSystemName ())
            .withParameter ("is_64", juce::SystemStats::isOperatingSystem64Bit () == true ? "1" : "0")
            .withParameter ("comp_name", juce::SystemStats::getComputerName ())
            .withParameter ("logon_name", juce::SystemStats::getLogonName ())
            .withParameter ("user_region", getPrivateString (juce::SystemStats::getUserRegion ()))
            .withParameter ("display_language", juce::SystemStats::getDisplayLanguage ())
            .withParameter ("logical_cores", getPrivateInt (juce::SystemStats::getNumCpus ()))
            .withParameter ("physical_cores", "" + juce::SystemStats::getNumPhysicalCpus ())
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