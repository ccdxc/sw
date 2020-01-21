import { Component, EventEmitter, Input, OnChanges, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';

/**
 *  an alert is like
 *  {
      "type": "Created",
      "object": {
        "kind": "Alert",
        "api-version": "v1",
        "meta": {
          "name": "c79bf224-b136-4edc-a3c6-bed60fbf4e01",
          "tenant": "default",
          "namespace": "default",
          "generation-id": "1",
          "resource-version": "853",
          "uuid": "ea29f2a8-60a7-43f8-90bd-ed71838c3c11",
          "creation-time": "2019-02-04T19:50:14.888656078Z",
          "mod-time": "2019-02-04T19:50:14.888662932Z",
          "self-link": "/configs/monitoring/v1/tenant/default/alerts/c79bf224-b136-4edc-a3c6-bed60fbf4e01"
        },
        "spec": {
          "state": "open"
        },
        "status": {
          "severity": "info",
          "source": {
            "component": "pen-cmd",
            "node-name": "node1"
          },
          "event-uri": "/events/v1/events/a849e59c-df64-4ea9-bd74-cda4e16895d7",
          "object-ref": null,
          "message": "Service pen-ntp stopped on node1",
          "reason": {
            "matched-requirements": [
              {
                "key": "Type",
                "operator": "in",
                "values": [
                  "ServiceStopped"
                ],
                "observed-value": "ServiceStopped"
              }
            ],
            "alert-policy-id": "eventstoalerts"
          },
          "acknowledged": null,
          "resolved": null
        }
      }
    }
 */

@Component({
  selector: 'app-alertlistitem',
  templateUrl: './alertlistitem.component.html',
  styleUrls: ['./alertlistitem.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertlistitemComponent implements OnInit, OnDestroy, OnChanges {
  public static ALERT_SOURCE_NAME_LINK: string = 'alertSourceNameLink';
  @Input() data;

  @Output() alertClick: EventEmitter<any> = new EventEmitter();
  @Output() alertSourceNameClick: EventEmitter<any> = new EventEmitter();


  alert;
  constructor() { }

  ngOnInit() {
  }

  ngOnDestroy() {
  }

  ngOnChanges() {
    if (this.data) {
    // add a UI_FIELD_PROPERTY to avoid constant function running in html template.
    this.alert = this.data;
    this.alert[AlertlistitemComponent.ALERT_SOURCE_NAME_LINK] = this.getAlertItemSourceNameLink();
    }
  }

  /**
   * build css class name for alert.
   * css file has rules like
   * .alertlistitem-icon-warning {..}
   * .alertlistitem-icon-info { .. }
   */
  getAlertItemIconClass() {
    return 'alertlistitem-icon-' + this.getAlertItemSeverity();
  }

  getAlertItemSeverity(): string {
    let severity = this.alert.status.severity;
    severity = severity.toLowerCase();
    return severity;
  }

  getAlertItemIconTooltip(): string {
    return 'Alert policy ' + this.alert.status.reason['alert-policy-id'] + ' reports ' + this.alert.status.message;
  }

  onAlertItemClick($event) {
    // comment this line out for now as we are not updating alert in alertlist panel.
    // this.alert.check = !this.alert.check;
    this.alertClick.emit(this.alert);
  }

  onAlertItemSourceNameClick() {
    this.alertSourceNameClick.emit();
  }

  getAlertItemSourceNameLink(): string {
    return Utility.getAlertSourceLink(this.alert);
  }

  getAlertItemSourceNameTooltip(): string {
    const cat = this.alert.status['object-ref'].kind;
    return 'Go to ' + (cat ? cat.toLowerCase() + ' ' : '') + 'details page';
  }
}
