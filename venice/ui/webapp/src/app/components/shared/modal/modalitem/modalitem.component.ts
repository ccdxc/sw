import { Component, Input, OnChanges, OnDestroy, OnInit, ViewEncapsulation, SimpleChanges } from '@angular/core';
import { Observable } from 'rxjs/Observable';
import { Subscription } from 'rxjs/Subscription';

/**
 * This component is for rendering an item that should have a progress icon.
 * It has two inputs, a status input and an observer which it will subscribe to and listen
 * for status events. It will try to use errorStateObserver observer first if it exists,
 * and default to the status input if it is null. These variables set the progress icon.
 */

export enum ModalItemStatus {
  INVALID = 'INVALID',
  VALID = 'VALID',
  DEFAULT = 'DEFAULT'
}

@Component({
  selector: 'app-modalitem',
  templateUrl: './modalitem.component.html',
  styleUrls: ['./modalitem.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ModalitemComponent implements OnInit, OnChanges, OnDestroy {

  @Input() status = '';
  @Input() observer: Observable<any>;

  icon: any;
  iconStyle: string;
  subscription: Subscription;

  constructor() {
  }

  ngOnInit() {
    this.updateStatus(this.status);
    this.subscribe();
  }

  subscribe() {
    if (this.observer != null) {
      this.subscription = this.observer.subscribe(
        (data) => {
          if (data.status != null) {
            this.updateStatus(data.status);
          }
        }
      );
    }
  }

  unsubscribe() {
    if (this.subscription != null) {
      this.subscription.unsubscribe();
    }
  }

  /* Handles if the observable we should listen for changes */
  ngOnChanges(changes: SimpleChanges) {
    if (changes.status != null) {
      this.updateStatus(this.status);
    }
    if (changes.errorStateObserver != null) {
      this.unsubscribe();
      this.subscribe();
    }
  }

  updateStatus(status: string) {
    if (status == null) {
      status = '';
    }
    status = status.toUpperCase();
    switch (status) {
      case ModalItemStatus.INVALID:
        this.icon = { name: 'clear' };
        this.iconStyle = '#DD3B09';
        break;
      case ModalItemStatus.VALID:
        this.icon = { name: 'done' };
        this.iconStyle = '#77A746';
        break;
      default:
        this.icon = {};
        this.iconStyle = '#dddddd';
    }
  }

  ngOnDestroy() {
    this.unsubscribe();
  }
}
