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


  @Input() header: String;
  @Input() icon: any;
  @Input() options: any;
  @Input() transparent: boolean;

  constructor() {
  }

  ngOnInit() {
    this.hasOptions = this.options != null;
    this._iconStyles = this._setIconStyles();
    if (this.icon == null) {
      this.icon = {};
    }
  }

  _setIconStyles() {
    if (this.icon == null) {
      return;
    }

    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-left': this.icon.margin.left,
    };
    if (this.icon.url) {
      styles['background-image'] = 'url(' + this.icon.url + ')';
    }
    return styles;
  }

  ngOnDestroy() {
  }

}
