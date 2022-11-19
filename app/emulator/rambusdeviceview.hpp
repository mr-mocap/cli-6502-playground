#ifndef RAMBUSDEVICEVIEW_HPP
#define RAMBUSDEVICEVIEW_HPP

#include <QObject>
#include "rambusdevice.hpp"


class RamBusDeviceView : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RamBusDevice *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int           page  READ page  WRITE setPage  NOTIFY pageChanged)
public:
    RamBusDeviceView();
   ~RamBusDeviceView() = default;

    /** Retrieve the underlying model.
     *
     *  @return A pointer to the underlying model
     */
    ///@{
    const RamBusDevice *model() const { return _model; }
          RamBusDevice *model()       { return _model; }
    ///@}

    /** Sets the underlying model of this view.
     *
     *  @param new_model The model to view
     */
    void setModel(RamBusDevice *new_model);

    /** Queries the current page to view.
     *
     *  @return The current page to view
     */
    int  page() const { return _page; }

    /** Sets which page number to view.
     *
     *  @param new_page The new page number to view
     */
    void setPage(int new_page);

signals:
    /** Emitted when the underlying model is set or reset.
     *
     *  @see model
     *  @see setModel
     */
    void modelChanged();

    /** Emitted when the current page is changed.
     *
     *  @see page
     *  @see setPage
     */
    void pageChanged();

    void memoryChanged(RamBusDevice::addressType address, uint8_t value);
private:
    RamBusDevice  *_model = nullptr;
    int            _page  = 0x00;

private slots:
    /** Catch the memoryChanged signal from @c RamBusDevice
     *
     *  @param address The address that was changed
     *  @param value   The value it was changed to
     */
    void onMemoryChanged(RamBusDevice::addressType address, uint8_t value);
};

#endif // RAMBUSDEVICEVIEW_HPP
