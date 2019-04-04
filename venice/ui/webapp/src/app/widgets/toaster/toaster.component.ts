import { animate, animateChild, query, state, style, transition, trigger } from '@angular/animations';
import { Component, OnInit, Input, ViewEncapsulation, OnDestroy } from '@angular/core';
import { DomHandler } from 'primeng/primeng';
import { ToastItem, Toast } from 'primeng/toast';
import { Utility } from '@app/common/Utility';
import { MessageService, Message } from '@app/services/message.service';
import { Subscription } from 'rxjs';

/**
 * The templates are almost completely unchanged from the primeng version except that
 * they pass a function for closing the toasteritem to the template. This allows us to create
 * a custom close button for the toaster notifications.
 */

@Component({
  selector: 'app-toaster',
  template: `
      <div #container [ngClass]="{'ui-toast ui-widget': true,
              'ui-toast-top-right': position === 'top-right',
              'ui-toast-top-left': position === 'top-left',
              'ui-toast-bottom-right': position === 'bottom-right',
              'ui-toast-bottom-left': position === 'bottom-left',
              'ui-toast-top-center': position === 'top-center',
              'ui-toast-bottom-center': position === 'bottom-center',
              'ui-toast-center': position === 'center'}"
              [ngStyle]="style" [class]="styleClass">
          <app-toasteritem *ngFor="let msg of messages; let i=index" [message]="msg" [index]="i" (onClose)="onMessageClose($event)" [template]="template" @toastAnimation></app-toasteritem>
      </div>
  `,
  animations: [
    trigger('toastAnimation', [
      transition(':enter, :leave', [
        query('@*', animateChild())
      ])
    ])
  ],
  providers: [DomHandler]
})
export class ToasterComponent extends Toast implements OnInit, OnDestroy {
  removeSubscription: Subscription;

  constructor(messageService: MessageService, domHandler: DomHandler) {
    // Casting to any since our message service redeclares private properties
    super(<any>messageService, domHandler);
  }

  addMessage(message) {
    let hasMessage = false;
    if (this.messages) {
      if (message.summary === Utility.VENICE_CONNECT_FAILURE_SUMMARY) {
        this.messages.some((m) => {
          if (m.summary === message.summary) {
            m.detail = message.detail;
            return true;
          }
          return false;
        });
        hasMessage = true;
      }
      hasMessage = this.messages.some((m) => {
        return message.summary + '-' + message.detail === m.summary + '-' + m.detail;
      });
    }
    if (!hasMessage) {
      this.messages = this.messages ? [...this.messages, ...[message]] : [message];
    }
  }

  ngOnInit() {
    this.messageSubscription = this.messageService.messageObserver.subscribe(messages => {
      if (messages) {
        const allMessages = null;
        if (messages instanceof Array) {
          const filteredMessages = messages.filter(m => this.key === m.key);
          filteredMessages.forEach((m) => {
            this.addMessage(m);
          });
        } else if (this.key === messages.key) {
          this.addMessage(messages);
        }

        if (this.modal && this.messages && this.messages.length) {
          this.enableModality();
        }
      }
    });

    this.clearSubscription = this.messageService.clearObserver.subscribe(key => {
      if (key) {
        if (this.key === key) {
          this.messages = null;
        }
      } else {
        this.messages = null;
      }

      if (this.modal) {
        this.disableModality();
      }
    });


    // Casting to our message service, has to be cast to any first
    // since there is no overlap as we don't extend Primeng's messageService
    this.removeSubscription = (<any>this.messageService as MessageService).removeObserver.subscribe((req) => {
      if (this.messages) {
        this.messages = this.messages.filter(m => {
          if (req.summary === m.summary &&
            (req.detail == null || req.detail === m.detail)) {
            return false;
          }
          return true;
        });
      }
    });
  }

  ngOnDestroy() {
    if (this.removeSubscription) {
      this.removeSubscription.unsubscribe();
    }
    super.ngOnDestroy();
  }
}

@Component({
  selector: 'app-toasteritem',
  template: `
      <div #container class="ui-toast-message ui-shadow" [@messageState]="'visible'"
          [ngClass]="{'ui-toast-message-info': message.severity == 'info','ui-toast-message-warn': message.severity == 'warn',
              'ui-toast-message-error': message.severity == 'error','ui-toast-message-success': message.severity == 'success'}"
              (mouseenter)="onMouseEnter()" (mouseleave)="onMouseLeave()">
          <div class="ui-toast-message-content">
              <a href="#" class="ui-toast-close-icon pi pi-times" (click)="onCloseIconClick($event)" *ngIf="message.closable !== false"></a>
              <ng-container *ngIf="!template">
                  <!-- THIS WILL NOT BE RENDERED, custom template used in appcontent.component.html -->
                  <span class="ui-toast-icon pi"
                      [ngClass]="{'pi-info-circle': message.severity == 'info', 'pi-exclamation-triangle': message.severity == 'warn',
                          'pi-times': message.severity == 'error', 'pi-check' :message.severity == 'success'}"></span>
                  <div class="ui-toast-message-text-content">
                      <div class="ui-toast-summary">{{message.summary}}</div>
                      <div class="ui-toast-detail">{{message.detail}}</div>
                  </div>
              </ng-container>
              <ng-container *ngTemplateOutlet="template; context: {$implicit: message, onClose: onCloseIconClickWrapper}"></ng-container>
          </div>
      </div>
  `,
  animations: [
    trigger('messageState', [
      state('visible', style({
        transform: 'translateY(0)',
        opacity: 1
      })),
      transition('void => *', [
        style({ transform: 'translateY(100%)', opacity: 0 }),
        animate('300ms ease-out')
      ]),
      transition('* => void', [
        animate(('250ms ease-in'), style({
          height: 0,
          opacity: 0,
          transform: 'translateY(-100%)'
        }))
      ])
    ])
  ],
  providers: [DomHandler]
})
export class ToasterItemComponent extends ToastItem implements OnInit {
  @Input() message: Message;
  onCloseIconClickWrapper;

  constructor() {
    super();
  }

  ngOnInit() {
    // Wrapping the function so that "this" is defined to be
    // this toaster item, and not the context from which its called.
    this.onCloseIconClickWrapper = (event) => {
      this.onCloseIconClick(event);
    };
  }

}
