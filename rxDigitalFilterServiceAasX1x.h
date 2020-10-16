/*
 ******************************************************************************
 ************************* Copyright ERICSSON AB 2018 *************************
 ******************************************************************************
 * The Copyright to the computer programs herein is the property of ERICSSON AB
 *
 * The programs may be used and/or copied only with the written permission from
 * ERICSSON AB or in accordance with the terms and conditions stipulated in
 * the agreement/contract under which the programs have been supplied.
 ******************************************************************************
 */
#pragma once

#include "rxDigitalFilterServiceIf.h"
#include "serviceLocatorIf.h"
#include "instanceToken.h"
#include "blockInterface/mcbFilter.h"
#include "blockInterface/decimationV2.h"
#include "blockInterface/srcFilter.h"
#include "blockInterface/ulChannelFilterV2.h"
#include <memory>
#include <array>
#include <sstream>

class RuError;
class ServiceCommonDataIf;


namespace Rx
{

// the doxygen command @ingroup below uses a group that is defined in sw/app/doxygen/dox/common/packages.dox

/**
 * @ingroup rxDigitalFilterServicePackage
 *
 * This class implements the DigitalFilter service for Xenon1x.
 */
class DigitalFilterServiceAasX1x : public DigitalFilterServiceIf
{
public:
    /**
     * Constructor that complies with ServiceFramework::ServiceFactory.
     *
     * @param err An object used to return error conditions in the absence of exceptions.
     * @param serviceLocator Will be used by BranchCtrl to locate all services that
     * are going to be used.
     * @param serviceCommonData Generic service data so that the instance of the BranchCtrl
     * can be created and inserted into BranchRepository.
     * @param instanceToken Driver instance token to connect antenna branch and driver instances,
     * see below for more info.
     *
     * Driver instanceToken is a way to connect driver instances and antenna
     * branches. The exact value of the InstanceToken is unknown by this class
     * and is encapsulated by Driver::InstanceToken. The values of InstanceToken
     * is typically something like the following:
     * Driver::InstanceToken::NoInstance = -1 , RxBranchA = 0, RxBranchB = 1, etc...
     * See documentation for Driver::getInstanceTokens() for more details.
     */
    DigitalFilterServiceAasX1x(RuError& err,
                               ServiceFramework::ServiceLocatorIf& serviceLocator,
                               const ServiceCommonDataIf& serviceCommonData,
                               Driver::InstanceToken instanceToken);
    virtual ~DigitalFilterServiceAasX1x();

    //@{
    // Implements Rx::BaseServiceIf.
    const std::string& getServiceVariant() const override;
    bool preInitializeService() override;
    bool postInitializeService() override;
    //@}

    //@{
    // Implements Rx::DigitalFilterServiceIf.
    virtual bool configureFilter(TrCarrierStandard carrierStandard,
                                 TrCarrierType carrierType,
                                 const std::vector<uint8_t>& filterBranchIdList,
                                 Cpri::CpriSlotLength carrierCpriSlotlength = Cpri::SLOT_LENGTH_INVALID,
                                 uint32_t subCarrierSpace = 0) override;

    virtual bool releaseFilter(const std::vector<uint8_t>& filterBranchIdList) override;

    virtual bool hasDecimationFilter(void) const override;
    virtual bool hasChannelFilter(void) const override;
    virtual bool hasFilterEngines(void) const override;
    virtual uint32_t readSfirCfgDelay(uint8_t filterBranchId) override;
    virtual void writeSfirCfgDelay(uint8_t filterBranchId, uint32_t delay);

    virtual bool reConfigureChannelFilter(TrCarrierType carrierType,
                                          TrPrbFreqOffset_t prbFreqOffset,
                                          bool useCpriFreqShift,
                                          const std::vector<uint8_t>& filterBranchIdList,
                                          Cpri::CpriSlotLength carrierCpriSlotlength = Cpri::SLOT_LENGTH_INVALID) override;
    //@}

private:
    static constexpr uint32_t NUMBER_OF_FILTERS{8u};

    /** Available types of downconversion. Values should agree with the DB parameter @c /ul/downConversionType. */
    enum class DownConversionType { HETERODYNE_SDC = 0, HOMODYNE = 1, DCT_UNKNOWN };

    /**
     * Filter types in filter branch, listed in the order they appear in the uplink signal path.
     * This order must agree with the order in the DB parameter @c /ul/CARRIERTYPE/iFilter in
     * xenonUlFiltersCoeff.txt. There are 8 filters: 6 decimation, 1 resampler, 1 channel filter.
     */
    enum class FilterEngine { FE_DEC2_0, FE_DEC2_1, FE_DEC2_2, FE_DEC2_3, FE_DEC2_4, FE_SRC, FE_DEC2_5, FE_SFIR };

    /**
     * Provides the correct initiation order of the filter resources in a branch.
     * The release order is in the opposite direction.
     * @param n An initiation step in the range 0 to NUMBER_OF_FILTERS.
     * @return A filter resource.
     */
    FilterEngine filterEngineInitOrder(unsigned n);

    bool configureMultiCarrierBlockFilter(void);
    bool configureDecimationFilter(const std::string& filterName, uint8_t decFilterId, uint8_t filterBranchId, unsigned& sampleRate);
    bool configureSrcFilter(const std::string& filterName, uint8_t filterBranchId, unsigned& sampleRate);
    bool configureSrcFilterCtsr(const std::string& filterName, uint8_t filterBranchId, const Cpri::CpriSampleRate cpriSampleRate);
    bool configureChannelFilter(const std::string& filterName, uint8_t filterBranchId, unsigned& sampleRate, bool isAdditionalGainInSFIR);

    std::vector<int32_t> getMultiCarrierBlockFilterCoefficients() const;
    std::vector<int32_t> getFilterCoefficients(const std::string& identifier) const;
    std::vector<int32_t> getFilterConfiguration(const std::string& identifier, unsigned int) const;
    std::vector<std::string> getFilterNameList(TrCarrierType carrierType) const;
    std::map<uint32_t, std::string> getAlternaticeChannelFilterNamesAndPassBandWidths(const std::string& channelFilterName) const;
    std::string selectAlternativeChannelFilterName(const std::map<uint32_t, std::string>& altChFilterNamesAndPassBandWidths, uint32_t wantedFilterBW) const;
    bool getSignalBandwidth(TrCarrierType carrierType, int32_t& signalBW);
    bool getAttachedPrbBwKhz(uint16_t& attachedPrbBwKhz);

    std::string m_serviceVariant;            //< contains the serviceVariant
    RxBranchId m_antennaBranchId;
    static bool m_registeredInSrvFactory;    //< Set by the Service Factory when the service class is registered
    Driver::InstanceToken m_instanceToken;

    /** Pointer to the HALI-1 block interface. */
    std::unique_ptr<Driver::BlockInterface::McbFilter> m_mcbFilter{nullptr};
    std::unique_ptr<Driver::BlockInterface::DecimationV2> m_decimationV2{nullptr};
    std::unique_ptr<Driver::BlockInterface::SrcFilter> m_srcFilter{nullptr};
    std::unique_ptr<Driver::BlockInterface::UlChannelFilterV2> m_channelFilterV2{nullptr};

    uint32_t m_multiCarrierBlockRate{0u};         ///< Output sampling rate from MCB.

    friend inline std::ostringstream& operator<<(std::ostringstream& os, const Rx::DigitalFilterServiceAasX1x::DownConversionType& enumValue)
    {
        static const std::map<Rx::DigitalFilterServiceAasX1x::DownConversionType, std::string> conversions
        {
            {Rx::DigitalFilterServiceAasX1x::DownConversionType::HETERODYNE_SDC, std::string("HETERODYNE_SDC")},
            {Rx::DigitalFilterServiceAasX1x::DownConversionType::HOMODYNE, std::string("HOMODYNE")},
            {Rx::DigitalFilterServiceAasX1x::DownConversionType::DCT_UNKNOWN, std::string("DCT_UNKNOWN")},
        };
        CommFW::streamEnum(os, enumValue, conversions);
        return os;
    }
};

inline bool DigitalFilterServiceAasX1x::hasDecimationFilter() const
{
    /* Yes, Xenon has 6 decimation filters per filter branch. */
    return true;
}

inline bool DigitalFilterServiceAasX1x::hasChannelFilter() const
{
    /* Yes, Xenon has a loadable channel filter in each filter branch. */
    return true;
}

inline bool DigitalFilterServiceAasX1x::hasFilterEngines() const
{
    /* Yes, in Xenon all filter types in the filter branch are cascadable for better performance. */
    /* But no, we lie about it because we don't support cascading at this time. */
    return false;
}
}

