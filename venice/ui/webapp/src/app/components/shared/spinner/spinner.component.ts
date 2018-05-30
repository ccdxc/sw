import { Component, OnInit } from '@angular/core';

/**
 * Creates a spinner that should be used while widgets are loading data.
 */
@Component({
  selector: 'app-spinner',
  templateUrl: './spinner.component.html',
  styleUrls: ['./spinner.component.scss']
})
export class SpinnerComponent implements OnInit {

  constructor() { }

  ngOnInit() {
  }

}
