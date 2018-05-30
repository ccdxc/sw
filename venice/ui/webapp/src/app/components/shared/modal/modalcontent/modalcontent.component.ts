import { Component, ViewEncapsulation } from '@angular/core';

/**
 * This component is for rendering content in the body that should not have a
 * progress indicator. It adds a margin on the left to avoid conflicting with the
 * progress bar as well as creating indentation for multiple nested ModalcontentComponents.
 */

@Component({
  selector: 'app-modalcontent',
  templateUrl: './modalcontent.component.html',
  styleUrls: ['./modalcontent.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ModalcontentComponent {

  constructor() {
  }
}
