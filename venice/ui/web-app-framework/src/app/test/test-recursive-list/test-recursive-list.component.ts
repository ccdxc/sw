import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'app-test-recursive-list',
  templateUrl: './test-recursive-list.component.html',
  styleUrls: ['./test-recursive-list.component.css']
})
export class TestRecursiveListComponent implements OnInit {
  public data: any = [
    {
      title: 'childless',
      children: []
    },
    {
      title: 'great grandparent',
      children: [
        {
          title: 'childless grandsibiling',
          children: []
        },
        {
          title: 'grandparent',
          children: [
            {
              title: 'childless sibiling',
              children: []
            },
            {
              title: 'another childless sibiling',
              children: []
            },
            {
              title: 'parent',
              children: [
                {
                  title: 'child',
                  children: []
                },
                {
                  title: 'another child',
                  children: []
                },
              ]
            },
            {
              title: 'another parent',
              children: [
                {
                  title: 'child',
                  children: []
                },
              ]
            },
          ]
        },
        {
          title: 'another grandparent',
          children: [
            {
              title: 'parent',
              children: [
                {
                  title: 'child',
                  children: []
                },
                {
                  title: 'another child',
                  children: []
                },
                {
                  title: 'a third child',
                  children: []
                },
                {
                  title: 'teen mother',
                  children: [
                    {
                      title: 'accident',
                      children: []
                    },
                  ]
                },
              ]
            },
          ]
        },
      ]
    },
  ];
  constructor() { }

  ngOnInit() {
  }

  onItemSelect(item) {
    console.log('TestRecursiveListComponent.onItemSelect() ' + item);
  }

}
