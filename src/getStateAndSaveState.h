    /**
     * @brief This is supposed to save the license data... but I haven't seen an
     * implementation that puts code in this function, so I don't know if it needs
     * code or not, and when I have code in it, it doesn't work right
     * 
     * @param data 
     */
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

    /**
     * @brief This is supposed to return the license data, but like I said for saveState(),
     * I haven't seen this implemented and I don't know if it even needs to be implemented or not.
     * 
     * @return juce::String 
     */
    juce::String getState ()
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