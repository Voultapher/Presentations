import React from 'react';
import PropTypes from 'prop-types';

import { flatbuffers } from './flatbuffers';
import { Strawpoll } from './strawpoll_generated.js';

import './App.css';

function ResultDescription(props) {
  return (
    <div style={{ marginBottom: 4, overflow: 'hidden' }}>
      <div style={{ float: 'left' }}>{props.text}</div>
      <div style={{ float: 'right', fontSize: '0.8em', paddingRight: '0.5em'}}>
        {props.percent.toFixed(2)}%
        ({props.votes} votes)
      </div>
    </div>
  );
}

ResultDescription.propTypes = {
  text: PropTypes.string.isRequired,
  percent: PropTypes.number.isRequired,
  votes: PropTypes.number.isRequired
};

const barColorsA = [ // eslint-disable-line no-unused-vars
  '#4b3947',
  '#e3c17d',
  '#f9f2e5',
  '#9e8e85'
];

const barColorsB = [ // eslint-disable-line no-unused-vars
  '#9eb4db',
  '#756a92',
  '#494165',
  '#2e205c',
  '#22144f'
];

const barColorsC = [ // eslint-disable-line no-unused-vars
  '#ffd0b0',
  '#ffa996',
  '#e6857c',
  '#d1626d',
  '#b54b63'
];

function ResultBar(props) {
  return (
    <div className={'ResultBar'}>
      <div style={{
        width: `${props.width.toFixed(2)}%`,
        height: '100%',
        backgroundColor: props.barColor
      }} />
    </div>
  );
}

ResultBar.propTypes = {
  width: PropTypes.number.isRequired,
  barColor: PropTypes.string.isRequired
};

function BarChart(props) {
  props.answers.sort((a, b) => (a.votes < b.votes));

  const total = props.answers.reduce((sum, an) => (sum + an.votes), 0);
  return (
    <div className={'BarChart'}>
      {props.answers.map((an, i) => {
        const width = (an.votes / total) * 100;
        return (
          <div key={an.text} style={{ paddingBottom: '1em' }}>
            <ResultDescription
              text={an.text}
              percent={width}
              votes={an.votes}
            />
            <ResultBar
              width={width}
              barColor={props.barColors[i % props.barColors.length]}
            />
          </div>
        );
      })}
      <span style={{ fontSize: '0.9em' }}>Total Votes: {total}</span>
    </div>
  );
}

BarChart.propTypes = {
  answers: PropTypes.arrayOf(PropTypes.shape({
    text: PropTypes.string.isRequired,
    votes: PropTypes.number.isRequired
  })).isRequired,
  barColors: PropTypes.arrayOf(PropTypes.string).isRequired
};

BarChart.defaultProps = {
  barColors: barColorsB
};

const testState = {
  title: "What is the meaning of life?",
  answers: [
    {
      text: "Option A",
      votes: 12
    },
    {
      text: "There is none, so it doesn't matter",
      votes: 17
    },
    {
      text: "42",
      votes: 9
    },
    {
      text: "You think to much",
      votes: 5
    }
  ]
};

class StrawPoll extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      title: '',
      options: []
    };
  }

  componentDidMount() {
    this.setupWebSocketCommunication();
  }

  setupWebSocketCommunication() {
    this.socket = new WebSocket(this.props.apiUrl);
    this.socket.binaryType = 'arraybuffer';

    this.socket.addEventListener('open', this.fetchPoll);
    this.socket.addEventListener('message', this.handleServerResponse);
    this.socket.addEventListener('close', this.handleDisconnect);
  }

  fetchPoll = (event) => {
    const builder = new flatbuffers.Builder(1024);
    Strawpoll.Response.startResponse(builder);
    Strawpoll.Response.addType(builder, Strawpoll.RequestType.Poll);
    builder.finish(Strawpoll.Response.endResponse(builder));

    this.socket.send(builder.asUint8Array());
  }

  handleServerResponse = (event) => {
    //console.log("Server sent: ", event);

    const buf = new flatbuffers.ByteBuffer(new Uint8Array(event.data));
    const response = Strawpoll.Response.getRootAsResponse(buf);

    switch(response.type()) {
      case Strawpoll.ResponseType.Poll:
        this.updatePoll(response.poll());
        break;
      case Strawpoll.ResponseType.Votes:
        this.updateResult(response.result());
        break;
      case Strawpoll.ResponseType.Error:
        console.error("Error: ", response.error());
        break;
      default:
        console.error("Invalid response type: ", response.type());
    };
  }

  updatePoll = (poll) => {
    this.setState({
      title: poll.title(),
      options: Array.apply(null, { length: poll.optionsLength() })
      .map((v, i) => ({
        text: poll.options(i)
      }))
    });
  }

  updateResult = (result) => {
    this.setState((prevState) => ({
      options: prevState.options.map((option, i) => ({
        text: option.text,
        votes: result.votes(i)
      }))
    }));
  }

  handleDisconnect = (event) => {
    console.error("WebSocket closed");
  }

  render() {
    return (
      <div>
        <h1>{this.state.title}</h1>
        <BarChart answers={this.state.options} barColors={barColorsC} />
      </div>
    );
  }
};

StrawPoll.propTypes = {
  apiUrl: PropTypes.string.isRequired
};

class App extends React.Component {
  render() {
    return (
      <div className="App">
        <StrawPoll apiUrl={'ws://localhost:3003'} />
      </div>
    );
  }
};

export default App;
