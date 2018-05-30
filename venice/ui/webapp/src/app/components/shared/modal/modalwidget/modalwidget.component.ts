import { Component, ViewEncapsulation } from '@angular/core';

/**
 * This component sets the css classes needed for the modal widgets.
 * It contains 3 ng-contents that have selectors for modal-header, modal-body, and modal-footer.
 */

@Component({
  selector: 'app-modalwidget',
  templateUrl: './modalwidget.component.html',
  styleUrls: ['./modalwidget.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ModalwidgetComponent {

  constructor() {
  }
}
