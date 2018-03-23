import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PagebodyComponent } from './pagebody.component';

describe('PagebodyComponent', () => {
  let component: PagebodyComponent;
  let fixture: ComponentFixture<PagebodyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PagebodyComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PagebodyComponent);
    component = fixture.componentInstance;
    component.icon = {
      margin: {
        top: '5px',
        left: '5px',
        url: 'test.com'
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
