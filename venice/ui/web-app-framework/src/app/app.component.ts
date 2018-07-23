import { Component } from '@angular/core';
import { Router } from '@angular/router';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {
  constructor(
    private _router: Router
  ) {

  }

  public data: any = [
    {
      title: 'plotly',
      children: []
    },
    {
      title: 'barchart',
      children: []
    },
    {
      title: 'menu',
      children: []
    },
    {
      title: 'repeater',
      children: []
    },
    {
      title: 'uniquerepeater',
      children: []
    },
    {
      title: 'tabs',
      children: []
    }

  ];

  onItemSelect(item) {
    console.log('AppComponent.onItemSelect() ' + item);
    this._router.navigate([item.title]);
  }
}
