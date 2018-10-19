import { animate, animateChild, query, state, style, transition, trigger } from '@angular/animations';
import { Component, OnInit } from '@angular/core';
import { DomHandler, MessageService } from 'primeng/primeng';
import { ToastItem, Toast } from 'primeng/toast';

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
export class ToasterComponent extends Toast {
  constructor(messageService: MessageService, domHandler: DomHandler) {
    super(messageService, domHandler);
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
