import { Component, Input, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';

/**
 * Component for styling the content holder for each page.
 */
@Component({
  selector: 'app-pagebody',
  templateUrl: './pagebody.component.html',
  styleUrls: ['./pagebody.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class PagebodyComponent implements OnInit, OnDestroy {
  hasOptions: Boolean;
  _iconStyles: any;

  @Input() title: String;
  @Input() icon: any;
  @Input() options: any;

  constructor() {
  }

  ngOnInit() {
    this.title = 'Workloads header';
    this.hasOptions = this.options != null;

    this._iconStyles = this._setIconStyles();
  }

  _setIconStyles() {
    if (this.icon == null) {
      return;
    }

    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-left': this.icon.margin.left,
      'background-image': 'url(' + this.icon.url + ')',
    };
    return styles;
  }

  ngOnDestroy() {
  }

}
