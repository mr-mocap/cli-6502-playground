#include "rambusdeviceview.hpp"


RamBusDeviceView::RamBusDeviceView()
    :
    QObject(nullptr)
{
}

void RamBusDeviceView::setModel(RamBusDevice *new_model)
{
    if (new_model != _model)
    {
        if (_model)
        {
            _model->disconnect(_model, &RamBusDevice::memoryChanged,
                               this,   &RamBusDeviceView::onMemoryChanged);
        }
        _model = new_model;

        if (new_model)
        {
            new_model->connect(new_model, &RamBusDevice::memoryChanged,
                               this,      &RamBusDeviceView::onMemoryChanged);

            // Let's go ahead and fill in the content to display...
            //generateContent();
        }
        emit modelChanged();
    }
}

void RamBusDeviceView::setPage(int new_page)
{
    if (new_page != _page)
    {
        _page = new_page;
        emit pageChanged();
    }
}

void RamBusDeviceView::onMemoryChanged(RamBusDevice::addressType address, uint8_t value)
{
    emit memoryChanged(address, value);
}
