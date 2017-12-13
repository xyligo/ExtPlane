#include "flightgeardatasource.h"
#include <QDebug>
#include <QThread>
#include "../extplane-server/datarefs/floatdataref.h"

FlightGearDataSource::FlightGearDataSource() : DataSource() {
    connect(&tcpClient, &BasicTcpClient::tcpClientConnected,
            this, &FlightGearDataSource::sessionOpened);
    connect(&tcpClient, &BasicTcpClient::receivedLine,
            this, &FlightGearDataSource::readLine);
    connect(&tcpClient, &BasicTcpClient::networkError,
            this, &FlightGearDataSource::gotNetworkError);

    refMap.insert("sim/cockpit2/gauges/indicators/heading_electric_deg_mag_pilot", "/instrumentation/magnetic-compass/indicated-heading-deg");
    refMap.insert("sim/cockpit2/gauges/indicators/airspeed_kts_pilot", "/instrumentation/airspeed-indicator/indicated-speed-kt");
    refMap.insert("sim/cockpit2/gauges/indicators/pitch_vacuum_deg_pilot", "/instrumentation/attitude-indicator/indicated-pitch-deg");
    refMap.insert("sim/cockpit2/gauges/indicators/roll_vacuum_deg_pilot", "/instrumentation/attitude-indicator/indicated-roll-deg");
    refMap.insert("sim/flightmodel/misc/h_ind", "/instrumentation/altimeter/indicated-altitude-ft");
    refMap.insert("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "/instrumentation/altimeter/setting-inhg");
    refMap.insert("sim/flightmodel/position/vh_ind", "/instrumentation/vertical-speed-indicator/indicated-speed-fpm");
    refMap.insert("sim/cockpit2/gauges/indicators/slip_deg", "/instrumentation/slip-skid-ball/indicated-slip-skid");
    // Please add more here

    tcpClient.setLineEnding("\r\n");
    setHelpText("Start FlightGear with --telnet=5401 to allow access to it's properties. \nSee http://wiki.flightgear.org/Telnet_usage for more info. \nTrying to connect to it..");
}

void FlightGearDataSource::connectToSource()
{
    tcpClient.connectToHost("localhost" , 5401);
}

DataRef *FlightGearDataSource::subscribeRef(QString name)
{
    qDebug() << Q_FUNC_INFO << name;
    if(refMap.contains(name)) {
        QString fgRef = refMap.value(name);
        FloatDataRef *newRef = new FloatDataRef(this, name, 0);
        floatRefs.append(newRef);
        tcpClient.writeLine("subscribe " + fgRef);
        return newRef;
    }
    return nullptr;
}

void FlightGearDataSource::unsubscribeRef(DataRef *ref)
{
    qDebug() << Q_FUNC_INFO << ref->name();
    tcpClient.writeLine("unsubscribe " + refMap.value(ref->name()));
    refMap.remove(ref->name());
    ref->deleteLater();
}

void FlightGearDataSource::updateDataRef(DataRef *ref)
{
}

void FlightGearDataSource::changeDataRef(DataRef *ref)
{
}

void FlightGearDataSource::keyStroke(int keyid)
{
}

void FlightGearDataSource::buttonPress(int buttonid)
{
}

void FlightGearDataSource::buttonRelease(int buttonid)
{
}

void FlightGearDataSource::command(QString &name, extplaneCommandType type)
{
}

void FlightGearDataSource::sessionOpened()
{
    setHelpText(QString("Connected to FlightGear at %1:%2").arg(tcpClient.peerName()).arg(tcpClient.peerPort()));
    tcpClient.writeLine("data");
}

void FlightGearDataSource::readLine(QString line)
{
    if(line.contains("=")) {
        QStringList splitted = line.split('=');
        QString fgRef = splitted[0];
        QString valueStr = splitted[1];
        for(FloatDataRef *ref : floatRefs) {
            if(refMap.value(ref->name()) == fgRef) {
                ref->updateValue(valueStr.toFloat());
            }
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Unknown:" << line;
    }

}

void FlightGearDataSource::gotNetworkError(QString errstring)
{
    emit sourceNetworkError(errstring);
}